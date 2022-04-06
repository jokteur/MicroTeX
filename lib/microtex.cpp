#include "microtex.h"

#include "core/formula.h"
#include "otf/fontsense.h"
#include "macro/macro.h"
#include "utils/exceptions.h"
#include "utils/string_utils.h"
#include "render/builder.h"

using namespace std;
using namespace microtex;

namespace microtex {

struct Config {
  bool isInited;
  std::string defaultMainFontFamily;
  std::string defaultMathFontName;
  bool renderGlyphUsePath;
};

static Config MICROTEX_CONFIG{false, "", "", false};

} // namespace microtex

Config* MicroTeX::_config = &microtex::MICROTEX_CONFIG;

std::string MicroTeX::version() {
  return std::to_string(MICROTEX_VERSION_MAJOR) + "."
         + std::to_string(MICROTEX_VERSION_MINOR) + "."
         + std::to_string(MICROTEX_VERSION_PATCH);
}

#ifdef HAVE_AUTO_FONT_FIND

struct InitVisitor {

  FontMeta operator()(const FontSrc* src) {
    auto meta = FontContext::addFont(*src);
    if (!meta.isMathFont) {
      throw ex_invalid_param("'" + meta.name + "' is not a math font!");
    }
    return meta;
  }

  FontMeta operator()(const string& name) {
    fontsenseLookup();
    if (!FontContext::isMathFontExists(name)) {
      throw ex_invalid_param("Math font '" + name + "' does not exists!");
    }
    return FontContext::mathFontMetaOf(name);
  }

  FontMeta operator()(const InitFontSenseAuto& sense) {
    auto mathFont = fontsenseLookup();
    if (!mathFont.has_value()) {
      throw ex_invalid_param("No math font found by font-sense.");
    }
    return mathFont.value();
  }
};

FontMeta MicroTeX::init(const Init& init) {
  if (_config->isInited) return {};
  auto meta = std::visit(InitVisitor(), init);
  _config->defaultMathFontName = meta.name;
  _config->isInited = true;
  NewCommandMacro::_init_();
  return meta;
}

#endif // HAVE_AUTO_FONT_FIND

FontMeta MicroTeX::init(const FontSrc& mathFontSrc) {
  if (_config->isInited) return {};
  auto meta = FontContext::addFont(mathFontSrc);
  if (!meta.isValid()) {
    throw ex_invalid_param("'" + meta.name + "' is not a math font!");
  }
  _config->defaultMathFontName = meta.name;
  _config->isInited = true;
  NewCommandMacro::_init_();
  return meta;
}

bool MicroTeX::isInited() {
  return _config->isInited;
}

void MicroTeX::release() {
  MacroInfo::_free_();
  NewCommandMacro::_free_();
}

FontMeta MicroTeX::addFont(const FontSrc& src) {
  auto meta = FontContext::addFont(src);
  if (meta.isMathFont && _config->defaultMathFontName.empty()) {
    _config->defaultMathFontName = meta.name;
  }
  if (!meta.isMathFont && _config->defaultMainFontFamily.empty()) {
    _config->defaultMainFontFamily = meta.family;
  }
  return meta;
}

bool MicroTeX::setDefaultMathFont(const std::string& name) {
  if (!FontContext::isMathFontExists(name)) return false;
  _config->defaultMathFontName = name;
  return true;
}

bool MicroTeX::setDefaultMainFont(const std::string& family) {
  if (family.empty() || FontContext::isMainFontExists(family)) {
    _config->defaultMainFontFamily = family;
    return true;
  }
  return false;
}

std::vector<std::string> MicroTeX::mathFontNames() {
  return FontContext::mathFontNames();
}

std::vector<std::string> MicroTeX::mainFontFamilies() {
  return FontContext::mainFontFamilies();
}

void MicroTeX::overrideTexStyle(bool enable, TexStyle style) {
  RenderBuilder::overrideTexStyle(enable, style);
}

bool MicroTeX::hasGlyphPathRender() {
#ifdef HAVE_GLYPH_RENDER_PATH
  return true;
#else
  return false;
#endif
}

void MicroTeX::setRenderGlyphUsePath(bool use) {
#if GLYPH_RENDER_TYPE == 0
  _config->renderGlyphUsePath = use;
#endif
}

bool MicroTeX::isRenderGlyphUsePath() {
#if GLYPH_RENDER_TYPE == 0
  return _config->renderGlyphUsePath;
#elif GLYPH_RENDER_TYPE == 1
  return true;
#else
  return false;
#endif
}

Render* MicroTeX::parse(
  const string& latex, float width, float textSize, float lineSpace, color fg,
  bool fillWidth, const string& mathFontName, const string& mainFontFamily
) {
  Formula formula(latex);
  const auto isInline = !startsWith(latex, "$$") && !startsWith(latex, "\\[");
  const auto align = isInline ? Alignment::left : Alignment::center;
  Render* render = RenderBuilder()
    .setStyle(isInline ? TexStyle::text : TexStyle::display)
    .setTextSize(textSize)
    .setMathFontName(
      mathFontName.empty()
      ? _config->defaultMathFontName
      : mathFontName
    )
    .setMainFontName(
      mainFontFamily.empty()
      ? _config->defaultMainFontFamily
      : mainFontFamily
    )
    .setWidth({width, UnitType::pixel}, align)
    .setFillWidth(!isInline && fillWidth)
    .setLineSpace({lineSpace, UnitType::pixel})
    .setForeground(fg)
    .build(formula);
  return render;
}
