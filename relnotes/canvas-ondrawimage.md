SkCanvas::drawImage() now calls into drawImageRect(). The onDrawImage() virtual is still defined for
backwards compatibility but is never called.
