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

// Main numeric type
type Ratio struct {
	Num   int
	Total int
	Delta []int
}

func (this Ratio) Divide() string {
	return fmt.Sprintf("%.6f", float64(this.Num)/float64(this.Total))
}

func (this Ratio) GT(other Ratio) bool {
	return float64(this.Num)/float64(this.Total) >
		float64(other.Num)/float64(other.Total)
}

func (this *Ratio) Add(other Ratio) {
	if other.Num > 0 {
		this.Num += other.Num
		this.Total += other.Total
		this.Delta = []int{}
	}
}

type CalculatedDelta struct {
	Memory      float64
	Performance float64
	Disk        float64
	Count       int
	Total       int
	Graphemes   Ratio
	SoftBreaks  Ratio
	HardBreaks  Ratio
	Whitespaces Ratio
	Words       Ratio
	Controls    Ratio
	Data        ParsedData
}

func (this *CalculatedDelta) Add(other CalculatedDelta) {
	this.Performance += other.Performance
	this.Memory += other.Memory
	this.Disk += other.Disk
	this.Total += other.Total
	this.Count += 1
	this.Graphemes.Add(other.Graphemes)
	this.SoftBreaks.Add(other.SoftBreaks)
	this.HardBreaks.Add(other.HardBreaks)
	this.Whitespaces.Add(other.Whitespaces)
	this.Words.Add(other.Words)
	this.Controls.Add(other.Controls)
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

// Row type
type Row struct {
	Names    []string
	Text     string
	Delta    CalculatedDelta
	Children []Row
}

func NewImpl(impl string) *Row {
	return &Row{Names: []string{impl}, Children: []Row{}}
}

func NewLocale(impl string, locale string) *Row {
	return &Row{Names: []string{impl, locale}, Children: []Row{}}
}

func NewSize(names []string, size string) *Row {
	return &Row{Names: append(names, size), Children: []Row{}}
}

func NewRow(names []string, file string, text string, delta CalculatedDelta) *Row {
	return &Row{Names: append(names, file), Text: text, Delta: delta, Children: []Row{}}
}

func (this Row) FormattedText() string {
	formatted := this.Text
	shift := 0
	for i, t := range this.Delta.SoftBreaks.Delta {
		if i > 0 {
			if t < 0 {
				t = -1 * t
			}
			formatted = fmt.Sprintf("%s<br><b>%6d:</b> %s", formatted[:t+shift], t, formatted[t+shift:])
			shift += 9
		}
	}
	return formatted
}

func (this *Row) Add(child Row) {
	this.Delta.Add(child.Delta)
	this.Children = append(this.Children, child)
}

func (this Row) Name() string {
	names := ""
	for i, name := range this.Names {
		if i > 0 {
			names += "."
		}
		names += name
	}
	return names
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

func (this Row) Level() string {
	return fmt.Sprintf("level%d", len(this.Names)-1)
}

func (this Row) IsImplementation() bool {
	return len(this.Names) == 1
}

func (this Row) IsFile() bool {
	return len(this.Names) == 4
}

type WebPage struct {
	Title   string
	Heading string
	Rows    []Row
}

func addImpl(web *WebPage, impl Row) {
	web.Rows = append(web.Rows, impl)
}

func parseFile(path string) (ParsedData, error) {
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

	result.Graphemes = split(lines[2], " ")
	result.SoftBreaks = split(lines[3], " ")
	result.HardBreaks = split(lines[4], " ")
	result.Whitespaces = split(lines[5], " ")
	result.Words = split(lines[6], " ")
	result.Controls = split(lines[7], " ")

	return result, nil
}

func compareLines(expected []int, actual []int) Ratio {

	var diff []int
	diff = append(diff, len(expected))

	e := 1
	a := 1
	for e < len(expected) || a < len(actual) {
		if e >= len(expected) {
			diff = append(diff, -1*actual[a])
			a += 1
		} else if a >= len(actual) {
			diff = append(diff, +1*expected[e])
			e += 1
		} else if actual[a] < expected[e] {
			diff = append(diff, -1*actual[a])
			a += 1
		} else if actual[a] > expected[e] {
			diff = append(diff, +1*actual[a])
			e += 1
		} else {
			a += 1
			e += 1
		}
	}

	if len(diff) > 1 {
		return Ratio{len(diff) - 1, len(expected), diff}
	} else {
		return Ratio{0, 1, diff}
	}
}

func compareData(expected ParsedData, actual ParsedData) CalculatedDelta {

	var delta CalculatedDelta

	delta.Graphemes = compareLines(expected.Graphemes, actual.Graphemes)
	delta.SoftBreaks = compareLines(expected.SoftBreaks, actual.SoftBreaks)
	delta.HardBreaks = compareLines(expected.HardBreaks, actual.HardBreaks)
	delta.Whitespaces = compareLines(expected.Whitespaces, actual.Whitespaces)
	delta.Words = compareLines(expected.Words, actual.Words)
	delta.Controls = compareLines(expected.Controls, actual.Controls)

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
			} else if len(rows) <= 2 {
				return errors.New(fmt.Sprintf("Wrong directory structure: %s\n", inputFile))
			} else {
				// Find the parent row
				//fmt.Printf("file %s\n", inputFile)
				parentIndex := findParentRow(&rows, tokens[outputIndex+3])
				parent := &rows[parentIndex]
				impl := parent.Names[0]

				// Read and parse the data
				textFile := strings.Replace(inputFile, "/output/"+impl, "/input", 1)
				textContent, err := os.ReadFile(textFile)
				check(err)

				if len(textContent) == 0 {
					fmt.Printf("Empty text file %s\n", inputFile)
					return nil
				}

				var actualData ParsedData
				actualData, err = parseFile(inputFile)
				if err != nil {
					return errors.New(fmt.Sprintf("Cannot parse output file %s: %s\n", inputFile, err.Error()))
				}

				var validationData ParsedData
				validationFile := strings.Replace(inputFile, "/output/"+impl, "/validation", 1)
				validationData, err = parseFile(validationFile)
				if err != nil {
					return errors.New(fmt.Sprintf("Cannot parse validation file%s: %s\n", validationFile, err.Error()))
				}

				// Compare the data
				var delta CalculatedDelta
				delta = compareData(validationData, actualData)
				row := NewRow(parent.Names, textFile, string(textContent), delta)
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
	t, err := template.ParseFiles("../html/index.html", "../html/scripts.html", "../html/tbody.html")
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
