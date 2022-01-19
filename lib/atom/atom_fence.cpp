#include "atom/atom_fence.h"
#include "atom/atom_row.h"
#include "box/box_single.h"
#include "box/box_group.h"
#include "box/box_factory.h"
#include "core/glue.h"
#include "env/env.h"

using namespace tex;

sptr<Box> MiddleAtom::createBox(Env& env) {
  if (_height == 0) return _placeholder;
  return tex::createVDelim(_sym, env, _height, true);
}

sptr<Box> FencedAtom::createBox(Env& env) {
  if (_base == nullptr) {
    return StrutBox::empty();
  }
  if (auto r = dynamic_cast<RowAtom*>(_base.get()); r != nullptr) {
    r->setBreakable(false);
  }

  const auto axis = env.axisHeight() * env.scale();
  const auto center = [axis](const sptr<Box>& b) {
    b->_shift = -(b->vlen() / 2 - b->_height) - axis;
  };

  const auto base = _base->createBox(env);
  center(base);
  const auto h = base->vlen();

  for (const auto& m : _m) {
    m->_height = h;
    auto b = m->createBox(env);
    center(b);
    b->_shift -= base->_shift;
    base->replaceFirst(m->_placeholder, b);
  }

  auto hbox = sptrOf<HBox>();

  if (!_l.empty() && _l != ".") {
    auto l = tex::createVDelim(_l, env, h, true);
    center(l);
    hbox->add(l);
    if (!base->isSpace()) {
      hbox->add(Glue::get(AtomType::opening, _base->leftType(), env));
    }
  }

  hbox->add(base);

  if (!_r.empty() && _r != ".") {
    if (!base->isSpace()) {
      hbox->add(Glue::get(_base->rightType(), AtomType::closing, env));
    }
    auto r = tex::createVDelim(_r, env, h, true);
    center(r);
    hbox->add(r);
  }

  return hbox;
}