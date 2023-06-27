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

type RangedData struct {
	Graphemes   []Range
	SoftBreaks  []Range
	HardBreaks  []Range
	Whitespaces []Range
	Words       []Range
	Controls    []Range
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

func (this Row) FormattedChunks(ranges []Range, name string, includeRange bool) []Chunk {
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
			results = append(results, Chunk{text, name, Range{r.Start, r.End, r.Type}})
			gap = r.End
		} else {
			results = append(results, Chunk{"\u200B", name, Range{r.Start, r.Start, r.Type}})
			gap = r.Start
		}
	}
	if gap < len(this.Text) {
		text := this.Text[gap:]
		results = append(results, Chunk{text, "", Range{gap, len(this.Text), ""}})
	}
	return results
}

func (this Row) FormattedGraphemes() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Graphemes, "grapheme", true)
}

func (this Row) FormattedSoftBreaks() []Chunk {
	return this.FormattedChunks(this.Delta.Data.SoftBreaks, "softBreak", false)
}

func (this Row) FormattedHardBreaks() []Chunk {
	return this.FormattedChunks(this.Delta.Data.HardBreaks, "hardBreak", false)
}

func (this Row) FormattedWords() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Words, "word", false)
}

func (this Row) FormattedWhitespaces() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Whitespaces, "whitespace", true)
}

func (this Row) FormattedControls() []Chunk {
	return this.FormattedChunks(this.Delta.Data.Controls, "control", true)
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

func (this Row) IsImplementation() bool {
	return len(this.Names) == 1
}

func (this Row) HasGraphemes() bool {
	return len(this.Delta.Data.Graphemes) > 0
}

func (this Row) HasSoftBreaks() bool {
	return len(this.Delta.Data.SoftBreaks) > 0
}

func (this Row) HasHardBreaks() bool {
	return len(this.Delta.Data.HardBreaks) > 0
}

func (this Row) HasWhitespaces() bool {
	return len(this.Delta.Data.Whitespaces) > 0
}

func (this Row) HasWords() bool {
	return len(this.Delta.Data.Words) > 0
}

func (this Row) HasControls() bool {
	return len(this.Delta.Data.Controls) > 0
}

func (this Row) GraphemeNum() int {
	return len(this.Delta.Data.Graphemes) - 1
}

func (this Row) SoftBreakNum() int {
	return len(this.Delta.Data.SoftBreaks) - 1
}

func (this Row) HardBreakNum() int {
	return len(this.Delta.Data.HardBreaks) - 1
}

func (this Row) WhitespaceNum() int {
	return len(this.Delta.Data.Whitespaces) - 1
}

func (this Row) WordNum() int {
	return len(this.Delta.Data.Words) - 1
}

func (this Row) ControlNum() int {
	return len(this.Delta.Data.Controls) - 1
}

type WebPage struct {
	Title   string
	Heading string
	Rows    []Row
}

func assignIds(children *[]Row, parentId string, parentNum string) {
	for i := range *children {
		(*children)[i].Num = fmt.Sprintf("%s%d", parentNum, i+1)
		(*children)[i].Id = fmt.Sprintf("%s%d", parentId, i+1)
		(*children)[i].ParentId = parentId
		assignIds(&(*children)[i].Children, (*children)[i].Id, (*children)[i].Num)
	}
}

func addImpl(web *WebPage, impl Row) {
	impl.Num = fmt.Sprintf("%d", len(web.Rows)+1)
	impl.Id = fmt.Sprintf("id%d", len(web.Rows)+1)
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

func compareLines(expected []int, actual []int, includeRange bool) (Ratio, []Range) {

	var diff []Range
	diff = append(diff, Range{len(actual), len(expected), ""})
	aLen := len(actual) - 1
	eLen := len(expected) - 1

	e := 1
	a := 1
	for e < eLen || a < aLen {
		a1 := actual[a]
		if includeRange && a < aLen-1 {
			a1 = actual[a+1]
		}
		e1 := expected[e]
		if includeRange && e < eLen-1 {
			e1 = expected[e+1]
		}

		if e >= eLen {
			diff = append(diff, Range{Abs(actual[a]), Abs(a1), "extra"})
			a += 1
		} else if a >= aLen {
			diff = append(diff, Range{Abs(expected[e]), Abs(e1), "missing"})
			e += 1
		} else if actual[a] < expected[e] {
			diff = append(diff, Range{Abs(actual[a]), Abs(a1), "extra"})
			a += 1
		} else if actual[a] > expected[e] {
			diff = append(diff, Range{Abs(expected[e]), Abs(e1), "missing"})
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

	delta.Graphemes, delta.Data.Graphemes = compareLines(expected.Graphemes, actual.Graphemes, true)
	delta.SoftBreaks, delta.Data.SoftBreaks = compareLines(expected.SoftBreaks, actual.SoftBreaks, false)
	delta.HardBreaks, delta.Data.HardBreaks = compareLines(expected.HardBreaks, actual.HardBreaks, false)
	delta.Whitespaces, delta.Data.Whitespaces = compareLines(expected.Whitespaces, actual.Whitespaces, true)
	delta.Words, delta.Data.Words = compareLines(expected.Words, actual.Words, true)
	delta.Controls, delta.Data.Controls = compareLines(expected.Controls, actual.Controls, false)
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
	for i >= start {
		if start > 0 {
			(*rows)[i-1].Add((*rows)[i])
		}
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
				if inputFile[0] == '.' {
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
					finishRows(&rows, 1)
					rows = append(rows, *NewLocale(rows[0].Names[0], tokens[outputIndex+2]))
				} else if len(tokens) == outputIndex+4 {
					// ~/datasets/output/icu/en/small
					finishRows(&rows, 2)
					rows = append(rows, *NewSize(rows[len(rows)-1].Names, tokens[outputIndex+3]))
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

				if len(parent.Children) > 10 {
					return nil
				}

				// Read and parse the data
				textFile := strings.Replace(inputFile, "/output/"+impl, "/input", 1)
				textContent, err := os.ReadFile(textFile)
				check(err)

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
				parent.Add(*row)
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
