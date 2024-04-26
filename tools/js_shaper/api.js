let utils = {
    css : function ( element, property ) {
        return window.getComputedStyle( element, null ).getPropertyValue( property );
    }
}
// Text unit types
const UnitType = Object.freeze({
    text: "text",
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

    constructor(/*UnitType*/unitType, /*Number*/start, /*Number*/end) {
        this.unitType = unitType;
        this.start = start;
        this.end = end;
    }
    unitType;
    start;
    end;
}

class Grapheme extends TextUnit {
    constructor(/*Number*/start, /*Number*/end) {
        super(UnitType.grapheme, start, end);
    }
}

class Word extends TextUnit {
    constructor(/*Number*/start, /*Number*/end) {
        super(UnitType.word, start, end);
    }
}

class TextRun extends TextUnit {
    constructor(/*Number*/start, /*Number*/end) {
        super(UnitType.run, start, end);
    }
}

class Line extends TextUnit {
    constructor(/*Number*/start, /*Number*/end) {
        super(UnitType.line, start, end);
    }
}

class GlyphRange {
    constructor(start, end) {
        this.start = start;
        this.end = end;
    }
    start;
    end;
}

// Glyph cluster: a range of glyphs that visualize a single grapheme
class GlyphCluster {
    constructor(/*Grapheme*/grapheme, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox) {
        this.grapheme = grapheme;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
    }
    grapheme;
    glyphRange;
    boundingBox;
}

// Set of glyphs making a word (or a part of a word if it's broken by lines)
class GlyphWord {
    constructor(/*Word*/text, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox) {
        this.text = text;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
    }
    text;
    glyphRange;
    boundingBox;
}

// Set of glyphs with the same text attributes (font name, font style, font size, text direction, decorations, ...)
class GlyphRun {
    constructor(/*TextRun*/text, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox, /*String*/font) {
        this.text = text;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
        this.font = font;
    }
    text;
    glyphRange;
    boundingBox;
    font;
}

// Set of glyphs placed on the same line (could be shuffled inside, but it's still one range)
class ShapedLine {
    constructor(/*Line*/text, /*GlyphRange*/glyphRange, /*DOMRect*/boundingBox) {
        this.text = text;
        this.glyphRange = glyphRange;
        this.boundingBox = boundingBox;
    }
    text;
    glyphRange;
    boundingBox;
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
        this.#shapedLines = [];
        this.#glyphRuns = [];
        this.#boundingBox = new DOMRect(0, 0, 0, 0);
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
        let textUnit = new TextUnit(UnitType.text, this.text.length, this.text.length + text.length);
        this.#fontStyleRanges.push(textUnit, fontStyle);
        this.text += text;
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

        this.#layoutPerformed = true;

        return true;
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
        let start = 0;
        for (let i = 0; i < this.#properties.length; ++i) {
            const property = this.#properties[i];
            const isWhitespaces = this.#properties[i] & Properties.whiteSpace;
            if (property & Properties.graphemeStart && isGrapheme) {
                // Finish the grapheme
                html += this.text.substring(start, i);
                html += "</span>";
                isGrapheme = false;
            }
            if ((property & Properties.wordStart) && isWord) {
                // Finish the word
                html += "</span>";
                isWord = false;
                // Grapheme does not cross the word edges
                console.assert(property && Properties.graphemeStart);
            }
            if (property & Properties.wordStart) {
                // Start the word
                html += "<span id='w" + w + "'" +
                            (isWhitespaces ? " class='whitespaces'" : "")
                            + " style='word-break: keep-all;'>";
                w += 1;
                isWord = true;
            }
            if (property & Properties.graphemeStart) {
                // Start the grapheme
                html += "<span id='g" + g + "'>";
                g += 1;
                isGrapheme = true;
                start = i;
            }
        }
        if (isWord) {
            // Finish the word
            html += "</span>";
            //isWord = false;
        }
        if (isGrapheme) {
            // Finish the grapheme
            html += this.text.substring(start);
            html += "</span>";
            //isGrapheme = false;
        }
        this.#div = document.createElement("div");
        this.#div.innerHTML = html;
        this.#div.style = "style='left:0px; top:0px;'";
        document.body.appendChild(this.#div);
    }

    /**
     * Extract all information from the browser corresponding it to the generated <span> structures
     */
    #extractInfo() {
        // Bounding box
        const measurements = this.#div.getClientRects();
        console.assert(measurements.length === 1);
        this.#boundingBox = measurements[0];

        // Graphemes and words
        const words = this.#div.children;
        let textIndex = 0;
        let glyphClusterIndex = 0;
        let glyphStartIndex = 0;
        let lineStart = 0;
        let lineRect = new DOMRect();
        let runStart = 0;
        let runRect = new DOMRect();
        let runFont = "";
        for (let word of words) {
            const graphemes = word.children;
            for (let grapheme of graphemes) {
                const rects = grapheme.getClientRects();
    
                // TODO: Deal with the zalgo text situation
                //   (when the first base glyph could havediacritics sitting
                //    on the left from it)
                if (textIndex === 0) {
                    // This is the beginning of a first line
                    lineRect = rects[0];
                    runRect = rects[0];
                } else if (rects[0].top > lineRect.bottom) {
                    // TODO: Deal with text direction, for now only assuming LTR
                    const start = this.#graphemes[lineStart];
                    const end = this.#graphemes[this.#graphemes.length - 1];
                    let line = new ShapedLine(
                        new Line(start.grapheme.start, end.grapheme.end),
                        new GlyphRange(start.glyphRange.start, end.glyphRange.end),
                        lineRect
                    );
                    this.#shapedLines.push(line);
                    // This is the beginning of a new line
                    lineStart = this.#graphemes.length;
                    lineRect = rects[0];
                } else {
                    // TODO: for now assuming LTR (trying to check for it)
                    console.assert(rects[0].left >= lineRect.left);
                    // Extend the line rect
                    lineRect = new DOMRect(
                        Math.min(lineRect.left, rects[0].left),
                        Math.min(lineRect.top, rects[0].top),
                        Math.max(lineRect.right, rects[0].right) - Math.min(lineRect.left, rects[0].left),
                        Math.max(lineRect.bottom, rects[0].bottom) - Math.min(lineRect.top, rects[0].top));
                }

                // TODO: In case the text run is longer than the line (normal case)
                //  we need to break it into few runs (one per line).
                //  It simplifies a lot of things...
                const font = utils.css(grapheme, 'font');
                if (font !== runFont) {
                    // End the previous run
                    if (textIndex !== 0) {
                        const start = this.#graphemes[runStart];
                        const end = this.#graphemes[this.#graphemes.length - 1];
                        this.#glyphRuns.push(new GlyphRun(
                            new TextRun(start.grapheme.start, end.grapheme.end),
                            new GlyphRange(start.glyphRange.start, end.glyphRange.end),
                            runRect,
                            font
                        ));
                    }
                    // Start the new run
                    runFont = font;
                    runStart = glyphClusterIndex;
                } else {
                    // Extend the run rect
                    runRect = new DOMRect(
                        Math.min(runRect.left, rects[0].left),
                        Math.min(runRect.top, rects[0].top),
                        Math.max(runRect.right, rects[0].right) - Math.min(runRect.left, rects[0].left),
                        Math.max(runRect.bottom, rects[0].bottom) - Math.min(runRect.top, rects[0].top));
                }
                
                // It should not be possible to place a single grapheme on two lines
                console.assert(rects.length === 1);
                this.#graphemes.push(new GlyphCluster(
                    new Grapheme(textIndex, textIndex + grapheme.innerText.length),
                    new GlyphRange(glyphClusterIndex, glyphClusterIndex + 1),
                    rects[0]
                ));
                textIndex += grapheme.innerText.length;
                glyphClusterIndex += 1;
            }

            // TODO: Deal with too long words (longer than a line)
            console.assert((glyphStartIndex + word.children.length) === glyphClusterIndex);
            this.#words.push(new GlyphWord(
                new Word(textIndex, textIndex + word.innerText.length),
                new GlyphRange(glyphStartIndex, glyphStartIndex + word.children.length),
                this.#merge(word.getClientRects())
            ));
            glyphStartIndex = glyphClusterIndex;
        }

        if (lineStart !== glyphClusterIndex) {
            // Add the last line
            const start = this.#graphemes[lineStart];
            const end = this.#graphemes[glyphClusterIndex - 1];
            let line = new ShapedLine(
                new Line(start.grapheme.start, end.grapheme.end),
                new GlyphRange(start.glyphRange.start, end.glyphRange.end),
                lineRect
            );
            this.#shapedLines.push(line);
        }

        if (runStart !== glyphClusterIndex) {
            // Add the last run
            const start = this.#graphemes[runStart];
            const end = this.#graphemes[glyphClusterIndex - 1];
            const font = utils.css(words[0], 'font');
            this.#glyphRuns.push(new GlyphRun(
                new TextRun(start.grapheme.start, end.grapheme.end),
                new GlyphRange(start.glyphRange.start, end.glyphRange.end),
                runRect,
                font
            ));
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
        var segments = segmenter.segment(this.text);
        for (const segment of segments) {
            const index = segment["index"];
            const len = segment["segment"]["length"];
            this.#properties[index] |= property;
            // Let's check for whitespaces
            const whitespaces = granularity === "grapheme" && this.isWhitespaces(this.text.substring(index, index + len));
            if (whitespaces) {
                for (let i = index; i < index + len; ++i) {
                    this.#properties[i] |= Properties.whiteSpace;
                }
            }
        }
    }
    
    /** Merge rects
     * @param {DOMRectList} rects
     * @returns {DOMRect}
     */
    #merge(rects) {
        if (rects.length === 0) {
            return null;
        }
        let left = rects[0].left;
        let right = rects[0].right;
        let top = rects[0].top;
        let bottom = rects[0].bottom;
        for (let rect of rects) {
           left = Math.min(left, rect.left);
           right = Math.max(right, rect.right);
           top = Math.min(top, rect.top);
           bottom = Math.max(bottom, rect.bottom);
        }
        return new DOMRect(left, top, right - left, bottom - top);
    }
    
    /** Detects the line start by glyph index
     *
     * @param {Number} index
     * @returns {Boolean}
     */
    #findLine(index) {
        const containsIndex = (line) => line.glyphRange.start <= index && line.glyphRange.end > index;
        return this.#shapedLines.findIndex(containsIndex);
    }
    
    /** Detects the run start by glyph index
     *
     * @param {Number} index
     * @returns {Boolean}
     */
    #findRun(index) {
        const containsIndex = (run) => run.glyphRange.start <= index && run.glyphRange.end > index;
        return this.#glyphRuns.findIndex(containsIndex);
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
    
    /**
     * Returns shaped text run information by the index
     * @param {Number} index Zero-based line index
     * @returns {GlyphRun} Run information
     */
    queryRun(index) {
        if (!this.#layoutPerformed) {
            alert("Cannot query information without performing layout");
            return null;
        }
        if (index < 0 && index >= this.#glyphRuns.count()) {
            return null;
        }
        return this.#glyphRuns[index];
    }

    /** Updates #tbody element with all the extracted information */
    printText() {
        console.log(this.#div.outerHTML);
        let tbody = document.getElementById("tbody");
        let w = 0;
        let currentLine = -1;
        let currentRun = -1;
        for (let word of this.#div.children) {
            
            let printGraphemes = false;
            let g = this.#words[w].glyphRange.start;
            for (let grapheme of word.children) {
                const l = this.#findLine(g);
                if (l !== currentLine) {
                    let tr = document.createElement("tr");
                    tr.innerHTML += "<th scope='row'>" + "line[" + l + "]" + "</th>";
                    tr.innerHTML += "<td></td>";
                    let rect = this.#shapedLines[l].boundingBox;
                    tr.innerHTML += "<td>" + rect.left.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.top.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.right.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.bottom.toFixed(2) + "</td>";
                    tbody.appendChild(tr);
                    currentLine = l;
                }
                const r = this.#findRun(g);
                if (r !== currentRun) {
                    let tr = document.createElement("tr");
                    tr.innerHTML += "<th scope='row'>" + "run[" + r + "]" + "</th>";
                    tr.innerHTML += "<td>" + this.#glyphRuns[r].font + "</td>";
                    let rect = this.#glyphRuns[r].boundingBox;
                    tr.innerHTML += "<td>" + rect.left.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.top.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.right.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.bottom.toFixed(2) + "</td>";
                    tbody.appendChild(tr);
                    currentRun = r;
                }

                if (g === this.#words[w].glyphRange.start) {
                    let tr = document.createElement("tr");
                    if (word.classList.contains("whitespaces")) {
                        tr.innerHTML += "<th scope='row'>" + "whitespaces[" + w
                            + "]" + "</th>";
                    } else if (word.children.length === 1) {
                        tr.innerHTML += "<th scope='row'>" + "word/grapheme["
                            + w + "/" + g + "]" + "</th>";
                    } else {
                        tr.innerHTML += "<th scope='row'>" + "word[" + w + "]"
                            + "</th>";
                        printGraphemes = true;
                    }
                    tr.innerHTML += "<td>" + word.innerText + "</td>";
                    let rect = this.#words[w].boundingBox;
                    tr.innerHTML += "<td>" + rect.left.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.top.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.right.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.bottom.toFixed(2) + "</td>";
                    tbody.appendChild(tr);
                }
                if (printGraphemes) {
                    let tr = document.createElement("tr");
                    tr.innerHTML += "<th scope='row'>" + "grapheme[" + g + "]" + "</th>";
                    tr.innerHTML += "<td>" + grapheme.innerText + "</td>";
                    let rect = this.#graphemes[g].boundingBox;
                    tr.innerHTML += "<td>" + rect.left.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.top.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.right.toFixed(2) + "</td>";
                    tr.innerHTML += "<td>" + rect.bottom.toFixed(2) + "</td>";
                    tbody.appendChild(tr);
                }
                g += 1;
            }

            w += 1;
        }
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
    #shapedLines;
    #glyphRuns;
}
