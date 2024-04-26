// Text unit types
const UnitType = Object.freeze({
    grapheme: "grapheme",
    word: "word",
    run: "run",
    line: "line"
    // possibly, "bidi" and "whitespaces"?
});

// Text unit properties
const Properties = Object.freeze({
    graphemeStart: 1,
    wordStart: 2,
    whiteSpace: 4,
    // Lots of other properties, don't know what we need at the moment
});

// Text unit: a unit of text (grapheme, word, text run, line)
class TextUnit {

    constructor(unitType, textRange) {
        this.unitType = unitType;
        this.textRange = textRange;
    }
}

class Grapheme extends TextUnit {
    constructor(/*TextRange*/textRange) {
        super(UnitType.grapheme, textRange);
    }
}

class Word extends TextUnit {
    constructor(textRange) {
        super(UnitType.word, textRange);
    }
}

class TextRun extends TextUnit {
    constructor(textRange) {
        super(UnitType.run, textRange);
    }
}

class Line extends TextUnit {
    constructor(textRange) {
        super(UnitType.line, textRange);
    }
}

class GlyphRange {
    constructor(start, end) {
        this.start = start;
        this.end = end;
    }
}

// Glyph cluster: a range of glyphs that visualize a single grapheme
class GlyphCluster {
    constructor(/*Grapheme*/grapheme, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox) {
        this.grapheme = grapheme;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
    }
}

// Set of glyphs making a word (or a part of a word if it's broken by lines)
class GlyphWord {
    constructor(/*TextRun*/text, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox) {
        this.text = text;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
    }
}

// Set of glyphs with the same text attributes (font name, font style, font size, text direction, decorations, ...)
class GlyphRun {
    constructor(/*TextRange*/textRange, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox) {
        this.textRange = text;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
    }
}

// Set of glyphs placed on the same line (could be shuffled inside, but it's still one range)
class ShapedLine {
    constructor(/*TextRange*/textRange, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox) {
        this.textRange = textRange;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
    }
}

// TODO: Implement multiple styles
class fontStyleRanges {
    constructor(/*TextUnit*/textRange, /*String*/fontStyle) {
        this.textRange = textRange;
        this.fontStyle = fontStyle;
    }
}

// Collecting text properties and glyph information
class Shaper {
    constructor() {
        this.#clearInputs();
        this.#clearOutputs();
    }

    #clearInputs() {
        this.text = String();
        this.#fontStyleRanges = [];
    }

    #clearOutputs() {
        this.#layoutPerformed = false;
        this.#properties = [];
        this.#graphemes = [];
        this.#words = [];
        this.#glyphClusters = [];
        this.#shapedLines = [];
        this.#boundingBox = DOMRect(0, 0, 0, 0);
    }

    /**
     * Adds styled text part to the paragraph
     * @param {String} text Text to add
     * @param {String} fontStyle Style of the text
     */
    addText(text, fontStyle) {
        if (this.#layoutPerformed) {
            alert("Cannot modify the text after layout");
            return;
        }
        let textUnit = TextUnit(this.text.count(), this.text.count() + text.count());
        this.#fontStyleRanges.push(textUnit, fontStyle);
    }

    /**
     * Performs the text layout
     * @param {Number} width
     * @returns {Boolean} Success of the operation
     */
    layout(width) {
        this.#clearOutputs();

        // TODO: deal with locales
        this.#extractSegments("grapheme", Properties.graphemeStart);
        this.#extractSegments("word", Properties.wordStart);

        // TODO: Here we extract all the information from the browser
        //   (corresponding it with the information from intl.segmenter, possibly)
        this.#generateSpanStructures();
        this.#extractInfo();

        return false;
    }

    /**
     * Generate <span> structures for all types of text units to pass it to the browser and get back
     * the shaped results
     */
    #generateSpanStructures() {
        let isGrapheme = false;
        let isWord = false;
        let html = "";
        let g = 0;
        let w = 0;
        for (let i = 0; i < this.#properties; ++i) {
            const property = this.#properties[i];

            if (property & Properties.wordStart && isWord) {
                // Finish the word
                html += "</span>";
                isWord = false;
                // Grapheme does not cross the word edges
                console.assert(property && Properties.graphemeStart);
            }
            if (property & Properties.graphemeStart && isGrapheme) {
                // Finish the grapheme
                html += "</span>";
                isGrapheme = false;
            }

            if (property & Properties.graphemeStart) {
                // Start the grapheme
                html += "<span id='g'" + g + ">";
                g += 1;
                isGrapheme = true;
            }
            if (property & Properties.wordStart) {
                // Start the word
                html += "<span id='w'" + w + ">";
                w += 1;
                isWord = true;
            }
        }
         if (isWord) {
            // Finish the word
            html += "</span style='word-break: keep-all;'>";
            //isWord = false;
        }
        if (isGrapheme) {
            // Finish the grapheme
            html += "</span>";
            //isGrapheme = false;
        }
        this.#div = document.createElement("div");
        this.#div.innerHTML = html;
    }

    /**
     * Extract all information from the browser corresponding it to the generated <span> structures
     */
    #extractInfo() {
        console.assert(this.#layoutPerformed);

        // Bounding box
        const measurements = this.#div.getClientRects();
        console.assert(measurements.length === 1);
        this.#boundingBox = measurements[0];

        // Graphemes and words
        const spans = this.#div.children;
        let textIndex = 0;
        let glyphClusterIndex = 0;
        let lineStart = 0;
        let lineRect = DOMRect();
        for (let span of spans) {
            const rects = child.getClientRects();

            // TODO: Deal with text direction, for now only assuming LTR
            if (rects[0].top > lineRect.bottom && textIndex > 0) {
                const start = this.#glyphClusters[lineStart];
                const end = this.#graphemes.back();
                let line = new glyphClusters(
                    TextRange(start.textRange.start, end.textRange.end),
                    GlyphRange(start.glyphRange.start, end.glyphRange.end),

                );
                this.#shapedLines.push(line);
                // This is the beginning of a new line
                lineStart = this.#graphemes.count();
                lineRect = rects[0];
            } else {
                // TODO: for now assuming LTR (trying to check for it)
                assert(rects[0].left > lineRect.left);
                // Extend the line rect
                lineRect.left = Math.min(lineRect.left, rects[0].left);
                lineRect.right = Math.max(lineRect.right, rects[0].right);
                lineRect.top = Math.min(lineRect.left, rects[0].left);
                lineRect.bottom = Math.max(lineRect.bottom, rects[0].bottom);
            }

            if (span.id[0] === 'g') {
                // It should not be possible to place a single grapheme on two lines
                console.assert(rects.length === 1);
                let grapheme = new GlyphCluster(
                    TextRange(textIndex, textIndex + span.innerHTML.length),
                    GlyphRange(glyphClusterIndex, glyphClusterIndex + 1),
                    rects[0]
                );
                this.#graphemes.push(grapheme);
                textIndex += span.innerHTML.length;
                glyphClusterIndex += 1;
            }

            if (span.id[0] === 'w') {
                // If a single word is placed on a few lines we have to pretend it's a few words
                // TODO: Deal with too long words
                console.assert(rects.length === 1);
                let word = new GlyphWord(
                    TextRange(textIndex, textIndex + span.innerHTML.length),
                    GlyphRange(glyphClusterIndex, glyphClusterIndex + span.children.length),
                    rects[0]
                );
                this.#words.push(word);
            }
        }

        // TODO: Runs (LTR|RTL)
    }

    /**
     * Extracts properties by the utf16 index in the text
     * (will return all properties that are actually applicable to the entire grapheme)
     * @param {Number} index
     * @return {Properties}
     */
    getProperties(index) {
        if (index < 0 || index >= this.#properties.count()) {
            return null;
        }
        return this.#properties[index];
    }

    /**
     * Check if the text consists of whitespaces only
     * @param {String} text
     * @return {Boolean} yes, it's whitespaces
     */
    isWhitespaces(text) {
        // TODO: Make it a proper hardcode
        for (let c of text) {
            if (c !== ' ') {
                return false;
            }
        }
        return true;
    }

    /**
     * Extract all the segments of a given type (possibly, with additional attributes)
     * @param {String} granularity
     * @param {String} property
     */
    #extractSegments(granularity, property) {
        var segmenter = new Intl["Segmenter"]("en", { "granularity": granularity });
        var segments = segmenter.segment(text);
        for (const segment of segments) {
            const index = segment["index"];
            const len = segment["segment"]["length"];
            // Let's check for whitespaces
            const whitespaces = granularity === "grapheme" && this.isWhitespaces(text.substring(index, index + len));
            for (let i = index; i < index + len; ++i) {
                this.#properties[i] |= property;
                if (whitespaces) {
                    this.#properties[i] |= Properties.whiteSpace;
                }
            }
        }
    }

    /* Querying the results */
    /**
     * Returns a paragraph measurements
     * @return {DOMRect} A bounding box for the entire paragraph's text
     */
    measurement() {
        if (!this.#layoutPerformed) {
            alert("Cannot query information without performing layout");
            return null;
        }
        return this.#boundingBox;
    }

    /**
     * Returns a text positions of grapheme beginnings
     * @returns {Array} Grapheme start position
     */
    queryGraphemes() {
        if (!this.#layoutPerformed) {
            alert("Cannot query information without performing layout");
            return null;
        }
        return this.#graphemes;
    }

    /**
     * Returns a text positions of word beginnings
     * @returns {Array} Word start position
     */
    queryWords() {
        if (!this.#layoutPerformed) {
            alert("Cannot query information without performing layout");
            return null;
        }
        return this.#words;
    }

    /**
     * Returns line information by line index
     * @param {Number} index Zero-based line index
     * @returns {ShapedLine} Line information
     */
    queryLine(index) {
        if (!this.#layoutPerformed) {
            alert("Cannot query information without performing layout");
            return null;
        }
        if (index < 0 && index >= this.#shapedLines.count()) {
            return null;
        }
        return this.#shapedLines[index];
    }

    // Input
    #fontStyleRanges;
    /*String*/text;
    /*Boolean*/#layoutPerformed;

    // Internal
    #div;

    // Output
    #properties;
    #boundingBox;
    #graphemes;
    #words;
    #glyphClusters;
    #shapedLines;
}
