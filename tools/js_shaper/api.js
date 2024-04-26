let utils = {
    css : function ( element, property ) {
        return window.getComputedStyle( element, null ).getPropertyValue( property );
    },

    extend: ( /*DOMRect*/rect1, /*DOMRect*/rect2) => {
        const left = Math.min(rect1.left, rect2.left);
        const right = Math.max(rect1.right, rect2.right);
        const top = Math.min(rect1.top, rect2.top);
        const bottom = Math.max(rect1.bottom, rect2.bottom);
        return new DOMRect(left, top, right - left, bottom - top);
    },
}

// Text unit properties
const TextDirection = Object.freeze({
    Undefined: "Undefined",
    LTR: "LTR",
    RTL: "RTL",
});

// Text unit properties
const Properties = Object.freeze({
    graphemeStart: 1,
    wordStart: 2,
    whiteSpace: 4,
    // Lots of other properties, don't know what we need at the moment
});

// Text unit: a unit of text (grapheme, word, text run, line)
class TextRange {
    constructor(/*Number*/start, /*Number*/end) {
        this.start = start;
        this.end = end;
    }
    start;
    end;
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
    constructor() {
        this.textRange = new TextRange();
        this.glyphRange = new GlyphRange();
        this.boundingBox = new DOMRect();
        this.text = "";
    }

    textRange;
    glyphRange;
    boundingBox;
    text;
}

// Set of glyphs making a word (or a part of a word if it's broken by lines)
class GlyphWord {
    constructor() {
        this.textRange = new TextRange();
        this.glyphRange = new GlyphRange();
        this.boundingBox = new DOMRect();
        this.text = "";
        this.isWhitespaces = false;
    }

    startFrom(/*GlyphCluster*/currentCluster, /*Boolean*/firstTime) {
        this.textRange.start = firstTime ? currentCluster.textRange.start : this.textRange.end;
        this.glyphRange.start = firstTime? currentCluster.glyphRange.start : this.glyphRange.end;
        this.boundingBox = currentCluster.boundingBox;
        this.text = currentCluster.text;
    }

    textRange;
    glyphRange;
    boundingBox;
    text;
    isWhitespaces;
}

// Set of glyphs with the same text attributes (font name, font style, font size, text direction, decorations, ...)
class GlyphRun {
    constructor() {
        this.textRange = new TextRange();
        this.glyphRange = new GlyphRange();
        this.wordRange = new GlyphRange();
        this.boundingBox = new DOMRect();
        this.textDirection = TextDirection.Undefined;
    }

    startFrom(/*GlyphCluster*/currentCluster, /*Boolean*/firstTime) {
        this.textRange.start = firstTime ? currentCluster.textRange.start : this.textRange.end;
        this.textRange.end = this.textRange.start;
        this.glyphRange.start = firstTime? currentCluster.glyphRange.start : this.glyphRange.end;
        this.glyphRange.end = this.glyphRange.start;
        this.wordRange.start = firstTime? currentCluster.glyphRange.start : this.wordRange.end;
        this.wordRange.end = this.wordRange.start;
        this.boundingBox = currentCluster.boundingBox;
    }

    textRange;
    glyphRange;
    wordRange;
    boundingBox;
    font;
    textDirection;
}

// Set of glyphs placed on the same line (could be shuffled inside, but it's still one range)
class GlyphLine {
    constructor() {
        this.textRange = new TextRange();
        this.glyphRange = new GlyphRange();
        this.wordRange = new GlyphRange();
        this.runRange = new GlyphRange();
        this.boundingBox = new DOMRect();
    }

    startFrom(/*GlyphCluster*/currentCluster, /*Boolean*/firstTime) {
        this.textRange.start = firstTime ? currentCluster.textRange.start : this.textRange.end;
        this.glyphRange.start = firstTime? currentCluster.glyphRange.start : this.glyphRange.end;
        this.wordRange.start = firstTime? currentCluster.glyphRange.start : this.wordRange.end;
        this.runRange.start = firstTime? currentCluster.glyphRange.start : this.runRange.end;
        this.boundingBox = currentCluster.boundingBox;
    }

    endAt(/*GlyphCluster*/currentCluster) {
        this.textRange.end = currentCluster.textRange.end;
        this.glyphRange.end = currentCluster.glyphRange.end;
    }

    textRange;
    glyphRange;
    wordRange;
    runRange;
    boundingBox;
}

// TODO: Implement multiple styles
class fontStyleRanges {
    constructor(/*TextRange*/textRange, /*String*/fontStyle) {
        this.textRange = textRange;
        this.fontStyle = fontStyle;
    }
    textRange;
    fontStyle;
}

// Collecting text properties and glyph information
class Shaper {
    constructor() {
        this.#clearInputs();
        this.#clearOutputs();
    }

    #clearInputs() {
        this.text = '';
        this.#fontStyleRanges = [];
    }

    #clearOutputs() {
        this.#layoutPerformed = false;
        this.#properties = [];
        this.#graphemes = [];
        this.#words = [];
        this.#runs = [];
        this.#lines = [];
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
        let textUnit = new TextRange(this.text.length, this.text.length + text.length);
        this.#fontStyleRanges.push(textUnit, fontStyle);
        this.text += text;
    }

    /**
     * Performs the text layout
     * @param {Number} width
     */
    layout(width) {
        // TODO: deal with locales
        this.#clearOutputs();
        this.#extractSegments("grapheme", Properties.graphemeStart);
        this.#extractSegments("word", Properties.wordStart);

        this.#div = document.createElement("div");
        this.#div.style = "width:{width}px; left: 0px; right: 0px; margin: 0px; padding: 0px; border: 0px; visibility: collapsed";
        document.body.appendChild(this.#div);

        this.#generateSpanStructures();
        this.#extractInfo();
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
            const isNewline = (i < this.#properties.length - 1) && (this.text.substring(i, i + 1) === '\n');
            if (isNewline) {
                console.assert(!isWord && !isGrapheme);
                html += "<br/>";
                continue;
            }
            if (property & Properties.wordStart) {
                // Start the word
                html += "<span id='w${w}'" + " class='" + (isWhitespaces ? "whitespaces " : "") +  "word'>";
                w += 1;
                isWord = true;
            }
            if (property & Properties.graphemeStart) {
                // Start the grapheme
                html += `<span id='g${g}' class='grapheme'>`;
                g += 1;
                isGrapheme = true;
                start = i;
            }
        }
        if (isGrapheme) {
            // Finish the grapheme
            html += this.text.substring(start);
            html += "</span>";
            //isGrapheme = false;
        }
        if (isWord) {
            // Finish the word
            html += "</span>";
            //isWord = false;
        }
        this.#div.innerHTML = html;
    }

    #extend(parent, glyphCluster) {
        parent.textRange.end = glyphCluster.textRange.end;
        parent.glyphRange.end = glyphCluster.glyphRange.end;
        parent.boundingBox = utils.extend(parent.boundingBox, glyphCluster.boundingBox);
    }

    /**
     * Extract all information from the browser corresponding it to the generated <span> structures
     */
    #extractInfo() {
        // Bounding box
        const measurements = this.#div.getClientRects();
        console.assert(measurements.length === 1);
        this.#boundingBox = measurements[0];

        let currentCluster = new GlyphCluster();
        let currentWord = new GlyphWord();
        let currentRun = new GlyphRun();
        let currentLine = new GlyphLine();

        let textIndex = 0;
        for (let word of this.#div.children) {
            if (word.tagName !== "SPAN") {
                // Skip <br/>
                continue;
            }

            // Start a new word
            for (let grapheme of word.children) {
                const graphemeRects = grapheme.getClientRects();
                console.assert(graphemeRects.length === 1);
                currentCluster.boundingBox = graphemeRects[0];

                if (textIndex === 0) {
                    // Initialize the new cluster, word, run and line
                    currentCluster.textRange = new TextRange(textIndex, textIndex + grapheme.innerText.length);
                    currentCluster.glyphRange = new GlyphRange(0, 1);
                    currentCluster.text = grapheme.innerText;

                    currentWord.startFrom(currentCluster, true);
                    currentRun.startFrom(currentCluster, true);
                    currentLine.startFrom(currentCluster, true);
                    textIndex += grapheme.innerText.length;
                }

                // Detect the line break.
                // Line break breaks also the run and the word (just to keep everything in order)
                // We also use the same code to initialize the structures (textIndex === 0)
                if (currentCluster.boundingBox.top >= currentLine.boundingBox.bottom) {
                    // Add collected word, run and line to the list
                    this.#words.push(currentWord);
                    this.#runs.push(currentRun);
                    this.#lines.push(currentLine);

                    // Start the new word, run and line
                    currentWord.startFrom(currentCluster, false);
                    currentRun.startFrom(currentCluster, false);
                    currentLine.startFrom(currentCluster, false);
                }

                // Check if the current grapheme has a different text direction (run break)
                // Run break should also break the word and we have to assert that
                const leftToRight = currentRun.boundingBox.right === currentCluster.boundingBox.right;
                const rightToLeft = currentRun.boundingBox.left === currentCluster.boundingBox.left;
                // The next grapheme could only be on the left (RTL) or on the right (LTR) from the current word
                console.assert(leftToRight !== rightToLeft || ((currentRun.glyphRange.start + 1) >= currentRun.glyphRange.end));
                const textDir = leftToRight ? TextDirection.LTR : TextDirection.RTL;
                if ((textIndex === 0) || (currentRun.textDirection !== textDir)) {
                    if (currentRun.textDirection !== TextDirection.Undefined) {
                        // Let's break the word (end the collected one and start another)
                        this.#words.push(currentWord);
                        currentWord.textRange.start = currentWord.textRange.end;
                        currentWord.glyphRange.start = currentWord.glyphRange.end;
                        currentWord.boundingBox = currentCluster.boundingBox;

                        // Let's end the previous run (and the run if it's not empty)
                        currentRun.textRange.end = textIndex;
                        currentRun.glyphRange.end = this.#graphemes.length;
                        this.#runs.push(currentRun);
                    }

                    // Let's start the new run
                    currentRun.textRange.start = currentRun.textRange.end;
                    currentRun.glyphRange.start = currentRun.glyphRange.end;
                    currentRun.boundingBox = currentCluster.boundingBox;
                    currentRun.textDirection = textDir;
                }

                // Finish the current grapheme
                currentCluster.textRange.end = textIndex + grapheme.innerText.length;
                currentCluster.glyphRange.end = this.#graphemes.length + 1;
                this.#graphemes.push(currentCluster);

                // Extend all the cursors: word, run and line
                this.#extend(currentWord, currentCluster);
                this.#extend(currentRun, currentCluster);
                this.#extend(currentLine, currentCluster);

                // Start a new grapheme
                currentCluster.textRange.start = currentCluster.textRange.end;
                currentCluster.glyphRange.start = currentCluster.glyphRange.end;

                // Increment textIndex here: the current position in the text
                textIndex += grapheme.innerText.length;
            }
            // Finish the current word, start the new one
            const wordRects = word.getClientRects();
            console.assert(wordRects.length === 1);
            console.assert(wordRects[0] === currentWord.boundingBox);
            currentWord.text = word.innerText;
            currentWord.isWhitespaces = word.classList.contains("whitespaces");
            console.assert(currentWord.wordText.end === textIndex);
            this.#words.push(currentWord);
            currentWord.startFrom(currentCluster, false);
        }

        // Add collected run and line to the list
        this.#runs.push(currentRun);
        this.#lines.push(currentLine);

        this.#layoutPerformed = true;
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
     * writing information into #properties array (one element per utf16)
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
        return this.shapedLines.findIndex(containsIndex);
    }
    
    /** Detects the run start by glyph index
     *
     * @param {Number} index
     * @returns {Boolean}
     */
    #findRun(index) {
        const containsIndex = (run) => run.glyphRange.start <= index && run.glyphRange.end > index;
        return this.glyphRuns.findIndex(containsIndex);
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
        if (index < 0 && index >= this.shapedLines.count()) {
            return null;
        }
        return this.#lines[index];
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
        if (index < 0 && index >= this.glyphRuns.count()) {
            return null;
        }
        return this.#runs[index];
    }

    /** Generate TR element
     * @param {HTMLElement} tbody
     * @param {String} unitType
     * @param {String} classes
     * @param {Number} index1
     * @param {Number} index2
     * @param {String} text
     * @param {DOMRect} rect
     * @param {DOMRect} lineRect
     */
    #generateRow(tbody, unitType, classes, index1, index2, text, rect, lineRect) {
        let tr = document.createElement("tr");
        tr.classList = classes;
        tr.innerHTML += "<th scope='row'>" + unitType +"[" + index1 + (index2 >= 0 ? "/" + index2 : "") + "]" + "</th>";
        tr.innerHTML += "<td>" + text + "</td>";

        let corrected = new DOMRect(rect.left - lineRect.left, rect.top - lineRect.top, rect.width, rect.height);
        tr.innerHTML += "<td>" + corrected.left.toFixed(2) + "</td>";
        tr.innerHTML += "<td>" + corrected.top.toFixed(2) + "</td>";
        tr.innerHTML += "<td>" + corrected.right.toFixed(2) + "</td>";
        tr.innerHTML += "<td>" + corrected.bottom.toFixed(2) + "</td>";
        tbody.appendChild(tr);
    }

    /** Updates #tbody element with all the extracted information */
    printTable() {

        console.log(this.#div.outerHTML);
        let tbody = document.getElementById("tbody");
        let lineCount = 0;
        let runCount = 0;
        let wordCount = 0;
        let clusterCount = 0;
        for (let line of this.#lines) {
            ++lineCount;
            this.#generateRow(tbody, "line", "line", lineCount, -1, "", line.boundingBox, line.boundingBox);
            for (let runIndex = line.runRange.start; runIndex < line.runRange.end; ++runIndex) {
                const run = this.#runs[runIndex];
                ++runCount;
                this.#generateRow(tbody, "run", "run", runCount, -1, run.textDirection, run.boundingBox, line.boundingBox);
                for (let wordIndex = run.wordRange.start; wordIndex < run.wordRange.end; ++wordIndex) {
                    const word = this.#words[wordIndex];
                    ++wordCount;
                    // Few cases: whitespaces or word == cluster
                    let classes = "word";
                    let secondCount = -1;
                    if (word.isWhitespaces) {
                        classes += " whitespaces";
                    }
                    if (word.glyphRange.start + 1 === word.glyphRange.end) {
                        classes += " grapheme";
                        secondCount = graphemeCount;
                    }
                    this.#generateRow(tbody, "word", classes, wordCount, secondCount, word.text, word.boundingBox, line.boundingBox);

                    if (word.glyphRange.start + 1 !== word.glyphRange.end) {
                        for (let clusterIndex = word.glyphRange.start; clusterIndex < word.glyphRange.end; ++clusterIndex) {
                            const cluster = this.#graphemes[clusterIndex];
                            ++clusterCount;
                            this.#generateRow(tbody, "grapheme", "grapheme", clusterCount, -1, cluster.text, cluster.boundingBox, line.boundingBox);
                        }
                    }
                }
            }
        }
        const table = document.getElementById("table");
        console.log(table.outerHTML);
    }

    // Input
    #fontStyleRanges;
    /*String*/text;
    /*Boolean*/#layoutPerformed;

    // Internal
    #div;               // The key div element containing <span> structures

    // Output
    #properties;        // Bit flags keeping all the information about graphemes, words, whitespaces and such per utf16 character
    #boundingBox;       // The entire paragraph bounding box

    #graphemes;          // List of all grapheme clusters in the logical order (ignoring RTL)
    #words;              // List of all words
    #runs;               // List of text runs broken by text direction
    #lines;              // List of all lines
}
