#ifndef MICROTEX_FONT_SRC_H
#define MICROTEX_FONT_SRC_H

#include "microtexexport.h"
#include "utils/types.h"
#include <vector>
#include <string>

namespace microtex {

class Otf;

/** Source to load font. */
class MICROTEX_EXPORT FontSrc {
public:
  /**
   * The font file (descriptor), may be empty if glyphs are drawn by graphical-paths.
   *
   * NOTICE: the 'fontFile' doesn't have to be a real font file, it could be a font id,
   * a font name, or something like that, because all font loading will be done on the
   * user side (you can preload the font resources and gives its id, name or something
   * can distinguish various fonts).
   */
  const std::string fontFile;

  explicit FontSrc(std::string fontFile);

  virtual sptr<Otf> loadOtf() const = 0;

  virtual ~FontSrc() = default;
};

/** Font source from file. */
class MICROTEX_EXPORT FontSrcFile : public FontSrc {
public:
  const std::string clmFile;

  explicit FontSrcFile(
    std::string clmFile,
    std::string fontFile = ""
  );

  sptr<Otf> loadOtf() const override;
};

/** Font source from data. */
class MICROTEX_EXPORT FontSrcData : public FontSrc {
public:
  const size_t len;
  const u8* data;

  FontSrcData(
    size_t len,
    const u8* data,
    std::string fontFile = ""
  );

  sptr<Otf> loadOtf() const override;
};

}

#endif //MICROTEX_FONT_SRC_H
