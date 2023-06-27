// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	//"bufio"
	"errors"
	"flag"
	"fmt"
	"html/template"
	"sort"

	//"math"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type Range struct {
	Start int
	End   int
	Type  string
}

// Main numeric type
type Ratio struct {
	Num   int
	Total int
}

type FloatRatio struct {
	Top    float64
	Bottom float64
}

func (this Ratio) Divide() string {
	if this.Num == 0 {
		return "        "
	} else {
		return fmt.Sprintf("%.6f", float64(this.Num)/float64(this.Total))
	}
}
func (this FloatRatio) Percents() string {
	if this.Top == this.Bottom {
		return "        "
	} else {
		return fmt.Sprintf("%.6f", (float64(this.Top)/float64(this.Bottom)-1)*100)
	}
}

func (this Ratio) GT(other Ratio) bool {
	return float64(this.Num)/float64(this.Total) >
		float64(other.Num)/float64(other.Total)
}

func (this *Ratio) Add(other Ratio) {
	if other.Num > 0 {
		this.Num += other.Num
		this.Total += other.Total
	}
}

type CalculatedDelta struct {
	Memory      float64
	Performance FloatRatio
	Disk        float64
	RowCount    int
	DiffCount   int
	Graphemes   Ratio
	SoftBreaks  Ratio
	HardBreaks  Ratio
	Whitespaces Ratio
	Words       Ratio
	Controls    Ratio
	Data        RangedData
}

func NewCalculatedDelta() CalculatedDelta {
	return CalculatedDelta{
		0.0,
		FloatRatio{0.0, 0.0},
		0.0,
		0,
		0,
		Ratio{0, 1},
		Ratio{0, 1},
		Ratio{0, 1},
		Ratio{0, 1},
		Ratio{0, 1},
		Ratio{0, 1},
		NewRangedData(),
	}
}

func (this *CalculatedDelta) Add(other CalculatedDelta) {
	this.Performance.Top += other.Performance.Top
	this.Performance.Bottom += other.Performance.Bottom
	this.Memory += other.Memory
	this.Disk += other.Disk
	this.RowCount += 1
	this.DiffCount += other.DiffCount
	this.Graphemes.Add(other.Graphemes)
	this.SoftBreaks.Add(other.SoftBreaks)
	this.HardBreaks.Add(other.HardBreaks)
	this.Whitespaces.Add(other.Whitespaces)
	this.Words.Add(other.Words)
	this.Controls.Add(other.Controls)
}

type RangeDataSet struct {
	Graphemes   []Range
	SoftBreaks  []Range
	HardBreaks  []Range
	Whitespaces []Range
	Words       []Range
	Controls    []Range
}
type RangedData struct {
	Missing RangeDataSet
	Extra   RangeDataSet
}

func NewRangedData() RangedData {
	return RangedData{}
}

type ParsedData struct {
	Count       int
	Time        float64
	Memory      float64
	Graphemes   []int
	SoftBreaks  []int
	HardBreaks  []int
	Whitespaces []int
	Words       []int
	Controls    []int
}

func NewParsedData() ParsedData {
	return ParsedData{}
}

// Row type
type Row struct {
	Id       string
	Num      string
	ParentId string
	Names    []string
	Text     string
	IsFile   bool
	Delta    CalculatedDelta
	Children []Row
}

func NewImpl(impl string) *Row {
	return &Row{Names: []string{impl}, IsFile: false, Delta: NewCalculatedDelta(), Children: []Row{}}
}

func NewLocale(impl string, locale string) *Row {
	return &Row{Names: []string{impl, locale}, IsFile: false, Delta: NewCalculatedDelta(), Children: []Row{}}
}

func NewSize(names []string, size string) *Row {
	return &Row{Names: append(names, size), IsFile: false, Delta: NewCalculatedDelta(), Children: []Row{}}
}

func NewRow(names []string, file string, text string, delta CalculatedDelta) *Row {
	return &Row{Names: append(names, file), IsFile: true, Text: text, Delta: delta, Children: []Row{}}
}

type Chunk struct {
	Text    string
	Classes string
	Indexes Range
}

func (this Row) FormattedChunks(ranges []Range, name string, chunkType string, includeRange bool) []Chunk {

	results := []Chunk{}
	gap := 0
	for i, r := range ranges {
		if i == 0 {
			continue
		}

		if r.Start > gap {
			text := this.Text[gap:r.Start]
			results = append(results, Chunk{text, "", Range{gap, r.Start, ""}})
		}
		if includeRange {
			text := this.Text[r.Start:r.End]
			if name == "whitespace" {
				corrected := ""
				for _, t := range text {
					if t == ' ' {
						corrected += "nbsp;"
					} else {
						corrected += string(t)
					}
				}
				text = corrected
			}
			results = append(results, Chunk{text, name, Range{r.Start, r.End, chunkType}})
			gap = r.End
		} else {
			results = append(results, Chunk{"\u200B", name, Range{r.Start, r.Start, chunkType}})
			gap = r.Start
		}
	}
	if gap < len(this.Text) {
		text := this.Text[gap:]
		results = append(results, Chunk{text, "", Range{gap, len(this.Text), ""}})
	}
	return results
}

func (this Row) FormattedMissingGraphemes() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Missing.Graphemes, "grapheme", "missing", true)
}

func (this Row) FormattedExtraGraphemes() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Extra.Graphemes, "grapheme", "extra", true)
}

func (this Row) FormattedMissingSoftBreaks() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Missing.SoftBreaks, "softBreak", "missing", false)
}

func (this Row) FormattedExtraSoftBreaks() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Extra.SoftBreaks, "softBreak", "extra", false)
}

func (this Row) FormattedMissingHardBreaks() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Missing.HardBreaks, "hardBreak", "missing", false)
}

func (this Row) FormattedExtraHardBreaks() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Extra.HardBreaks, "hardBreak", "extra", false)
}

func (this Row) FormattedMissingWords() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Missing.Words, "word", "missing", false)
}

func (this Row) FormattedExtraWords() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Extra.Words, "word", "extra", false)
}

func (this Row) FormattedMissingWhitespaces() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Missing.Whitespaces, "whitespace", "missing", true)
}

func (this Row) FormattedExtraWhitespaces() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Extra.Whitespaces, "whitespace", "extra", true)
}

func (this Row) FormattedMissingControls() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Missing.Controls, "control", "missing", false)
}

func (this Row) FormattedExtraControls() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Extra.Controls, "control", "extra", false)
}

func (this *Row) Add(child Row) {
	this.Delta.Add(child.Delta)
	this.Children = append(this.Children, child)
}

func (this Row) Name() string {
	if this.IsFile {
		return this.Names[len(this.Names)-1]
	} else {
		names := ""
		for i, name := range this.Names {
			if i > 0 {
				names += "."
			}
			names += name
		}
		return names
	}
}

func (this Row) ParentName() string {
	names := ""
	for i, name := range this.Names {
		if i == len(this.Names)-1 {
			break
		}
		if i > 0 {
			names += "."
		}
		names += name
	}
	return names
}

func (this Row) Implementation() string {
	return this.Names[0]
}

func (this Row) Level() string {
	return fmt.Sprintf("l%d", len(this.Names))
}

func (this Row) HasText() bool {
	return len(this.Text) != 0
}

func (this Row) HasChildren() bool {
	return len(this.Children) > 0
}

func (this Row) IsImplementation() bool {
	return len(this.Names) == 1
}

func (this Row) HasNoDifferences() bool {
	return len(this.Delta.Data.Missing.Graphemes) == 0 &&
		len(this.Delta.Data.Missing.SoftBreaks) == 0 &&
		len(this.Delta.Data.Missing.HardBreaks) == 0 &&
		len(this.Delta.Data.Missing.Words) == 0 &&
		len(this.Delta.Data.Missing.Whitespaces) == 0 &&
		len(this.Delta.Data.Missing.Controls) == 0 &&
		len(this.Delta.Data.Extra.Graphemes) == 0 &&
		len(this.Delta.Data.Extra.SoftBreaks) == 0 &&
		len(this.Delta.Data.Extra.HardBreaks) == 0 &&
		len(this.Delta.Data.Extra.Words) == 0 &&
		len(this.Delta.Data.Extra.Whitespaces) == 0 &&
		len(this.Delta.Data.Extra.Controls) == 0
}

func (this Row) Differences() int {
	return len(this.Delta.Data.Missing.Graphemes) +
		len(this.Delta.Data.Missing.SoftBreaks) +
		len(this.Delta.Data.Missing.HardBreaks) +
		len(this.Delta.Data.Missing.Words) +
		len(this.Delta.Data.Missing.Whitespaces) +
		len(this.Delta.Data.Missing.Controls) +
		len(this.Delta.Data.Extra.Graphemes) +
		len(this.Delta.Data.Extra.SoftBreaks) +
		len(this.Delta.Data.Extra.HardBreaks) +
		len(this.Delta.Data.Extra.Words) +
		len(this.Delta.Data.Extra.Whitespaces) +
		len(this.Delta.Data.Extra.Controls)
}

func (this Row) HasMissingGraphemes() bool {
	return len(this.Delta.Data.Missing.Graphemes) > 0
}

func (this Row) HasExtraGraphemes() bool {
	return len(this.Delta.Data.Extra.Graphemes) > 0
}

func (this Row) HasNoGraphemes() bool {
	return !this.HasMissingGraphemes() && !this.HasExtraGraphemes()
}

func (this Row) HasMissingSoftBreaks() bool {
	return len(this.Delta.Data.Missing.SoftBreaks) > 0
}

func (this Row) HasExtraSoftBreaks() bool {
	return len(this.Delta.Data.Extra.SoftBreaks) > 0
}

func (this Row) HasNoSoftBreaks() bool {
	return !this.HasMissingSoftBreaks() && !this.HasExtraSoftBreaks()
}

func (this Row) HasMissingHardBreaks() bool {
	return len(this.Delta.Data.Missing.HardBreaks) > 0
}

func (this Row) HasExtraHardBreaks() bool {
	return len(this.Delta.Data.Extra.HardBreaks) > 0
}

func (this Row) HasNoHardBreaks() bool {
	return !this.HasMissingHardBreaks() && !this.HasExtraHardBreaks()
}

func (this Row) HasMissingWhitespaces() bool {
	return len(this.Delta.Data.Missing.Whitespaces) > 0
}

func (this Row) HasExtraWhitespaces() bool {
	return len(this.Delta.Data.Extra.Whitespaces) > 0
}

func (this Row) HasNoWhitespaces() bool {
	return !this.HasMissingWhitespaces() && !this.HasExtraWhitespaces()
}

func (this Row) HasMissingWords() bool {
	return len(this.Delta.Data.Missing.Words) > 0
}

func (this Row) HasExtraWords() bool {
	return len(this.Delta.Data.Extra.Words) > 0
}

func (this Row) HasNoWords() bool {
	return !this.HasMissingWords() && !this.HasExtraWords()
}

func (this Row) HasMissingControls() bool {
	return len(this.Delta.Data.Missing.Controls) > 0
}

func (this Row) HasExtraControls() bool {
	return len(this.Delta.Data.Extra.Controls) > 0
}
func (this Row) HasNoControls() bool {
	return !this.HasMissingControls() && !this.HasExtraControls()
}

func (this Row) MissingGraphemeNum() int {
	return len(this.Delta.Data.Missing.Graphemes) - 1
}

func (this Row) ExtraGraphemeNum() int {
	return len(this.Delta.Data.Extra.Graphemes) - 1
}

func (this Row) MissingSoftBreakNum() int {
	return len(this.Delta.Data.Missing.SoftBreaks) - 1
}

func (this Row) ExtraSoftBreakNum() int {
	return len(this.Delta.Data.Extra.SoftBreaks) - 1
}

func (this Row) MissingHardBreakNum() int {
	return len(this.Delta.Data.Missing.HardBreaks) - 1
}

func (this Row) ExtraHardBreakNum() int {
	return len(this.Delta.Data.Extra.HardBreaks) - 1
}

func (this Row) MissingWhitespaceNum() int {
	return len(this.Delta.Data.Missing.Whitespaces) - 1
}

func (this Row) ExtraWhitespaceNum() int {
	return len(this.Delta.Data.Extra.Whitespaces) - 1
}

func (this Row) MissingWordNum() int {
	return len(this.Delta.Data.Missing.Words) - 1
}

func (this Row) ExtraWordNum() int {
	return len(this.Delta.Data.Extra.Words) - 1
}

func (this Row) MissingControlNum() int {
	return len(this.Delta.Data.Missing.Controls) - 1
}

func (this Row) ExtraControlNum() int {
	return len(this.Delta.Data.Extra.Controls) - 1
}

type WebPage struct {
	Title   string
	Heading string
	Rows    []Row
}

func assignIds(children *[]Row, parentId string, parentNum string) {
	for i := range *children {
		(*children)[i].Num = fmt.Sprintf("%s_%d", parentNum, i+1)
		(*children)[i].Id = fmt.Sprintf("%s_%d", parentId, i+1)
		(*children)[i].ParentId = parentId
		assignIds(&(*children)[i].Children, (*children)[i].Id, (*children)[i].Num)
	}
}

func addImpl(web *WebPage, impl Row) {
	impl.Num = fmt.Sprintf("%d", len(web.Rows)+1)
	impl.Id = fmt.Sprintf("id_%d", len(web.Rows)+1)
	impl.ParentId = ""
	assignIds(&impl.Children, impl.Id, impl.Num)
	web.Rows = append(web.Rows, impl)
}

func parseFile(path string, textLen int) (ParsedData, error) {
	// Time: float64
	// Memory: float64
	// Graphemes: n1 n2 ...
	// SoftBreaks: n1 n2 ...
	// HardBreaks: n1 n2 ...
	// Whitespaces: n1 n2 ...
	// Words: n1 n2 ...
	// Controls: n1 n2 ...
	content, err := os.ReadFile(path)
	check(err)

	var result ParsedData
	lines := strings.Split(string(content), "\n")
	if len(lines) < 8 {
		return result, errors.New("Wrong data format (number of lines)")
	}

	result.Time, err = strconv.ParseFloat(lines[0], 64)
	if err != nil {
		return result, errors.New("Wrong data format (time)")
	}
	result.Memory, err = strconv.ParseFloat(lines[1], 64)
	if err != nil {
		return result, errors.New("Wrong data format (memory)")
	}

	result.Graphemes = split(lines[2]+" "+strconv.Itoa(textLen), " ")
	result.SoftBreaks = split(lines[3]+" "+strconv.Itoa(textLen), " ")
	result.HardBreaks = split(lines[4]+" "+strconv.Itoa(textLen), " ")
	result.Whitespaces = split(lines[5]+" "+strconv.Itoa(textLen), " ")
	result.Words = split(lines[6]+" "+strconv.Itoa(textLen), " ")
	result.Controls = split(lines[7]+" "+strconv.Itoa(textLen), " ")

	return result, nil
}

func compareLines(expected []int, actual []int, includeRange bool, missing bool) (Ratio, []Range) {

	var diff []Range
	diff = append(diff, Range{len(actual), len(expected), ""})
	aLen := len(actual) - 1
	eLen := len(expected) - 1

	e := 1
	a := 1
	for e < eLen || a < aLen {
		a1 := actual[a]
		if includeRange && a < aLen {
			a1 = actual[a+1]
		}
		e1 := expected[e]
		if includeRange && e < eLen {
			e1 = expected[e+1]
		}

		if e >= eLen {
			if !missing {
				diff = append(diff, Range{Abs(actual[a]), Abs(a1), "extra"})
			}
			a += 1
		} else if a >= aLen {
			if missing {
				diff = append(diff, Range{Abs(expected[e]), Abs(e1), "missing"})
			}
			e += 1
		} else if actual[a] < expected[e] {
			if !missing {
				diff = append(diff, Range{Abs(actual[a]), Abs(a1), "extra"})
			}
			a += 1
		} else if actual[a] > expected[e] {
			if missing {
				diff = append(diff, Range{Abs(expected[e]), Abs(e1), "missing"})
			}
			e += 1
		} else {
			a += 1
			e += 1
		}
	}

	// TODO: keep the difference, too
	if len(diff) > 1 {
		return Ratio{len(diff) - 1, len(expected)}, diff
	} else {
		return Ratio{0, 1}, []Range{}
	}
}

func compareData(expected ParsedData, actual ParsedData) CalculatedDelta {

	var delta CalculatedDelta

	delta.Performance.Top = actual.Time
	delta.Performance.Bottom = expected.Time

	deltaGraphemes := Ratio{0, 0}
	deltaSoftBreaks := Ratio{0, 0}
	deltaHardBreaks := Ratio{0, 0}
	deltaWhitespaces := Ratio{0, 0}
	deltaWords := Ratio{0, 0}
	deltaControls := Ratio{0, 0}

	deltaGraphemes, delta.Data.Missing.Graphemes = compareLines(expected.Graphemes, actual.Graphemes, true, true)
	deltaSoftBreaks, delta.Data.Missing.SoftBreaks = compareLines(expected.SoftBreaks, actual.SoftBreaks, false, true)
	deltaHardBreaks, delta.Data.Missing.HardBreaks = compareLines(expected.HardBreaks, actual.HardBreaks, false, true)
	deltaWhitespaces, delta.Data.Missing.Whitespaces = compareLines(expected.Whitespaces, actual.Whitespaces, true, true)
	deltaWords, delta.Data.Missing.Words = compareLines(expected.Words, actual.Words, true, true)
	deltaControls, delta.Data.Missing.Controls = compareLines(expected.Controls, actual.Controls, false, true)

	delta.Graphemes.Add(deltaGraphemes)
	delta.SoftBreaks.Add(deltaSoftBreaks)
	delta.HardBreaks.Add(deltaHardBreaks)
	delta.Whitespaces.Add(deltaWhitespaces)
	delta.Words.Add(deltaWords)
	delta.Controls.Add(deltaControls)

	deltaGraphemes, delta.Data.Extra.Graphemes = compareLines(expected.Graphemes, actual.Graphemes, true, false)
	deltaSoftBreaks, delta.Data.Extra.SoftBreaks = compareLines(expected.SoftBreaks, actual.SoftBreaks, false, false)
	deltaHardBreaks, delta.Data.Extra.HardBreaks = compareLines(expected.HardBreaks, actual.HardBreaks, false, false)
	deltaWhitespaces, delta.Data.Extra.Whitespaces = compareLines(expected.Whitespaces, actual.Whitespaces, true, false)
	deltaWords, delta.Data.Extra.Words = compareLines(expected.Words, actual.Words, true, false)
	deltaControls, delta.Data.Extra.Controls = compareLines(expected.Controls, actual.Controls, false, false)

	delta.Graphemes.Add(deltaGraphemes)
	delta.SoftBreaks.Add(deltaSoftBreaks)
	delta.HardBreaks.Add(deltaHardBreaks)
	delta.Whitespaces.Add(deltaWhitespaces)
	delta.Words.Add(deltaWords)
	delta.Controls.Add(deltaControls)

	return delta
}

func printDifference(text string, diff []int) {
	count := diff[0]

	if len(diff) <= 1 {
		// No diff
	} else if (len(diff)-1)*10 < count {
		// Too small diff
		fmt.Printf("%d < %d:\n%s\n", (len(diff)-1)*10, count, text)
		return
	} else if count == 0 {
		// Too small string
		fmt.Printf("%d == 0:\n%s\n", count, text)
		return
	}
	first := Abs(diff[1])
	last := first + 10
	if last >= len(text) {
		last = len(text) - 1
	}
	fmt.Printf("Difference @%d:\n%s\n", first, text[:last])
}

func finishRows(rows *[]Row, start int) {
	if len(*rows) == 0 {
		return
	}
	i := len(*rows) - 1
	for i > start {
		(*rows)[i-1].Add((*rows)[i])
		i -= 1
	}

	if start > 0 {
		*rows = (*rows)[0:start]
	} else {
		*rows = (*rows)[0 : start+1]
	}
}

func findParentRow(rows *[]Row, name string) int {
	for i := range *rows {
		row := (*rows)[len(*rows)-1-i]
		if row.Names[len(row.Names)-1] == name {
			return len(*rows) - 1 - i
		}
	}
	return -1
}

func compareFiles(validationPath string, inputPath string, outputFile *os.File, removeNoise bool, web *WebPage) error {

	var rows []Row

	err := filepath.Walk(inputPath,
		func(inputFile string, info os.FileInfo, err error) error {
			if err != nil {
				fmt.Println(err)
				return err
			}
			tokens := strings.Split(inputFile, "/")
			outputIndex := -1
			for i, t := range tokens {
				if t == "output" {
					outputIndex = i
					break
				}
			}
			if outputIndex < 0 {
				return errors.New(fmt.Sprintf("Currently only supported directory structure: [...]/output/{implementation}/{locale}:\n%s\n", inputFile))
			}

			if info.IsDir() {
				if len(tokens) == outputIndex+1 {
					// ~/datasets/output
				} else if len(tokens) == outputIndex+2 {
					// ~/datasets/output/icu
					finishRows(&rows, 0)
					if len(rows) > 0 {
						addImpl(web, rows[0])
					}
					rows = []Row{*NewImpl(tokens[outputIndex+1])}
				} else if len(tokens) == outputIndex+3 {
					// ~/datasets/output/icu/en
					finishRows(&rows, 0)
					rows = append(rows, *NewLocale(rows[0].Names[0], tokens[outputIndex+2]))
				} else {
					fmt.Printf("skipping %s\n", inputFile)
					return nil
				}
			} else if len(rows) <= 1 {
				return errors.New(fmt.Sprintf("Wrong directory structure: %s\n", inputFile))
			} else {
				// Find the parent row
				//fmt.Printf("file %s\n", inputFile)
				parent := &rows[len(rows)-1]
				impl := parent.Names[0]
				/*
					if len(parent.Children) > 10 {
						return nil
					}
				*/
				// Read and parse the data
				textFile := strings.Replace(inputFile, "/output/"+impl, "/input", 1)
				textContent, err := os.ReadFile(textFile)
				check(err)
				/* For debugging
				if inputFile == "Sep5/output/libgrapheme/fa/page.10" {
					fmt.Printf("libgrapheme/fa/page.10\n")
				}
				*/
				if len(textContent) == 0 {
					fmt.Printf("Empty text file %s\n", inputFile)
					return nil
				}

				var actualData ParsedData
				actualData, err = parseFile(inputFile, len(textContent))
				if err != nil {
					return errors.New(fmt.Sprintf("Cannot parse output file %s: %s\n", inputFile, err.Error()))
				}

				var validationData ParsedData
				validationFile := strings.Replace(inputFile, "/output/"+impl, "/validation", 1)
				validationData, err = parseFile(validationFile, len(textContent))
				if err != nil {
					return errors.New(fmt.Sprintf("Cannot parse validation file%s: %s\n", validationFile, err.Error()))
				}

				// Compare the data
				var delta CalculatedDelta
				shortFileName := strings.TrimPrefix(inputFile, inputPath+"/")
				delta = compareData(validationData, actualData)
				row := NewRow(parent.Names, shortFileName, string(textContent), delta)
				if !row.HasNoDifferences() {
					parent.Add(*row)
					sort.Slice(parent.Children, func(i, j int) bool {
						return parent.Children[i].Differences() > parent.Children[j].Differences()
					})
					if len(parent.Children) > 10 {
						parent.Children = parent.Children[0:9]
					}
				}
			}
			return nil
		})
	finishRows(&rows, 0)
	if len(rows) > 0 {
		addImpl(web, rows[0])
	}
	return err
}

func main() {
	var (
		root        = flag.String("root", "~/datasets", "Folder (inputs for the table expected to be under <Folder>/output/>")
		removeNoise = flag.Bool("removeNoise", true, "Removes all unsignificant differences")
	)
	flag.Parse()
	if *root == "" {
		fmt.Println("Must set --root")
		flag.PrintDefaults()
	}
	expand(root)

	// Define the data to be used in the template
	web := WebPage{
		Title: "Comparison Table (accuracy, performance and disk memory)",
	}

	// Parse the template
	t, err := template.ParseFiles("../html/index.html", "../html/scripts.html", "../html/styles.html", "../html/tbody.html")
	check(err)

	// Create index.html
	indexPath := *root + "/index.html"
	indexFile, err := os.Create(indexPath)
	check(err)

	validationPath := *root + "/validation"
	outputPath := *root + "/output"
	err = compareFiles(validationPath, outputPath, indexFile, *removeNoise, &web)
	check(err)

	// Execute the template and write the result to index.html
	err = t.Execute(indexFile, web)
	check(err)
	indexFile.Close()
}
