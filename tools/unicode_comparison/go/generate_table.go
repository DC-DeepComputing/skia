// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
  //"bufio"
  "errors"
  "path/filepath"
  "io/ioutil"
	"flag"
	"fmt"
  "html/template"
	//"math"
	"os"
	"sort"
  "strings"
  "strconv"
)

// Helper functions
func Abs(x int) int {
    if x < 0 {
            return -x
    }
    return x
}

func check(e error) {
    if e != nil {
        fmt.Sprintf("Error: %s\n", e)
        panic(e)
    }
}

func expand(path *string) {
    if strings.HasPrefix(*path, "~/") {
        home, err := os.UserHomeDir()
        check(err)
        *path = filepath.Join(home, (*path)[2:])
    }
}

func split(str string, sep string)[]int {
    arr := strings.Split(str, sep)
    var result = []int {}
    for _, i := range arr {
        j, err := strconv.Atoi(i)
        if err != nil {
            return []int {}
        }
        result = append(result, j)
    }
    return result
}

func insert(a []int, index int, value int) []int {
    if len(a) == index { // nil or empty slice or after last element
        return append(a, value)
    }
    a = append(a[:index+1], a[index:]...) // index < len(a)
    a[index] = value
    return a
}

func min(a, b int) int {
    if a < b {
        return a
    }
    return b
}

// Data structures
type Ratio struct {
    Num int
    Total int
    Delta [] int
}

func (this Ratio) Divide () string {
    return fmt.Sprintf("%.6f", float64(this.Num)/float64(this.Total))
}

func (this Ratio) GT (other Ratio) bool {
    return float64(this.Num) / float64(this.Total) >
                float64(other.Num) / float64(other.Total)
}

func (this *Ratio) Add(other Ratio) {
    if other.Num > 0 {
        this.Num += other.Num
        this.Total += other.Total
        this.Delta = []int {}
    }
}

type CalculatedDelta struct {
    Memory float64
    Performance float64
    Disk float64
    Count int
    Total int
    Graphemes Ratio
    SoftBreaks Ratio
    HardBreaks Ratio
    Whitespaces Ratio
    Words Ratio
    Controls Ratio
    Data ParsedData
}

func (this *CalculatedDelta) Clean() {
    this.Performance = 0.0
    this.Memory = 0.0
    this.Disk = 0.0
    this.Total = 0
    this.Count = 0
    this.Graphemes = Ratio { 0, 1, [] int {} }
    this.SoftBreaks = Ratio { 0, 1, [] int {} }
    this.HardBreaks = Ratio { 0, 1, [] int {} }
    this.Whitespaces = Ratio { 0, 1, [] int {} }
    this.Words = Ratio { 0, 1, [] int {} }
    this.Controls = Ratio { 0, 1, [] int {} }
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
    Count int
    Time float64
    Memory float64
    Graphemes [] int
    SoftBreaks [] int
    HardBreaks [] int
    Whitespaces [] int
    Words [] int
    Controls [] int
}

// Single file data
type File struct {
    Name string
    Locale string
    Text string
    Delta CalculatedDelta
}

func NewFile(name string, loc string, text string, delta CalculatedDelta) *File {
    return &File { Name: name, Locale: loc, Text: text, Delta: delta }
}

func (this File) FormattedText() string {
    formatted := this.Text
    shift := 0
    for i, t := range this.Delta.SoftBreaks.Delta {
        if i > 0 {
            if t < 0 {
                t = -1 * t
            }
            formatted = fmt.Sprintf("%s<br><b>%6d:</b> %s", formatted[:t + shift], t, formatted[t + shift:])
            shift += 9
        }
    }
    return formatted
}

// Locale data
type Locale struct {
    Name string
    Delta CalculatedDelta
    Files [] File
}

func NewLocale(name string) *Locale {
    return &Locale { Name: name }
}

func (this *Locale) Clean() {
    this.Name = ""
    this.Delta.Clean()
    this.Files = [] File { }
}

func (this *Locale) Add(file File) {
    this.Delta.Add(file.Delta)
    this.Files = append(this.Files, file)
}

// Implementation data (with all the locales)
type Impl struct {
    Name string
    Locales [] Locale
    Delta CalculatedDelta
}

func NewImpl(name string) *Impl {
    return &Impl { Name: name }
}

func (this *Impl) Clean() {
    this.Name = ""
    this.Locales = [] Locale { }
    this.Delta.Clean()
}

func (this *Impl) addLocale(other *Locale) {

    sort.Slice(other.Files, func(i, j int) bool {
        return other.Files[i].Delta.SoftBreaks.GT(other.Files[j].Delta.SoftBreaks)
    })

    len := min(10, len(other.Files))
    other.Files = append([]File(nil), other.Files[:len]...)

    this.Locales = append(this.Locales, *other)
    this.Delta.Add(other.Delta)
    other.Clean();
}

type WebPage struct {
    Title string
    Heading string
    Rows [] Impl
}

func addImpl(web *WebPage, impl *Impl) {
    web.Rows = append(web.Rows, *impl)
    impl.Clean()
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
    content, err := ioutil.ReadFile(path)
    check(err)

    var result ParsedData
    lines := strings.Split(string(content), "\n")
    if (len(lines) < 8) {
        return result, errors.New("Wrong data format (number of lines)")
    }

    result.Time, err = strconv.ParseFloat(lines[0], 64)
    if (err != nil) {
        return result, errors.New("Wrong data format (time)")
    }
    result.Memory, err = strconv.ParseFloat(lines[1], 64)
    if (err != nil) {
        return result, errors.New("Wrong data format (memory)")
    }

    result.Graphemes = split(lines[2], " ")
    result.SoftBreaks = split(lines[3], " ")
    result.HardBreaks = split(lines[4], " ")
    result.Whitespaces = split(lines[5], " ")
    result.Words = split(lines[6], " ")
    result.Controls = split(lines[7], " ")

    return result, nil;
}

func compareLines(expected []int, actual []int) Ratio {

    var diff [] int
    diff = append(diff, len(expected))

    e := 1
    a := 1
    for e < len(expected) || a < len(actual) {
        if e >= len(expected) {
            diff = append(diff, -1 * actual[a])
            a += 1
        } else if a >= len(actual) {
            diff = append(diff, +1 * expected[e])
            e += 1
        } else if actual[a] < expected[e] {
            diff = append(diff, -1 * actual[a])
            a += 1
        } else if actual[a] > expected[e] {
            diff = append(diff, +1 * actual[a])
            e += 1
        } else {
            a += 1
            e += 1
        }
    }

    if len(diff) > 1 {
        return Ratio { len(diff) - 1, len(expected), diff }
    } else {
        return Ratio { 0, 1, diff }
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

func printDifference(text string, diff []int ) {
    count := diff[0]

    if len(diff) <= 1 {
        // No diff
    } else if (len(diff) - 1)*10 < count {
        // Too small diff
        fmt.Printf("%d < %d:\n%s\n", (len(diff) - 1)*10, count, text)
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

func compareFiles(validationPath string, inputPath string, outputFile *os.File, removeNoise bool, web *WebPage) error {

    var impl Impl
    var locale Locale
    impl.Clean()
    locale.Clean()

    err := filepath.Walk(inputPath,
    func(inputFile string, info os.FileInfo, err error) error {
        if err != nil {
            fmt.Println(err)
            return err
        }
        tokens := strings.Split(inputFile, "/")
        if info.IsDir() {
            outputIndex := -1
            for i, t := range tokens {
                if t == "output" {
                    outputIndex = i
                    break
                }
            }
            if outputIndex < 0 {
                return  errors.New(fmt.Sprintf("Currently only supported directory structure: [...]/output/{implementation}/{locale}:\n%s\n", inputFile))
            }

            if inputFile[0] == '.' {
                // ~/datasets/output
            } else if len(tokens) == outputIndex + 2 {
                fmt.Printf("Impl: %s\n", inputFile)
                if len(locale.Name) > 0 {
                    impl.addLocale(&locale)
                }
                if len(impl.Name) > 0 {
                    addImpl(web, &impl)
                }
                impl.Name = tokens[outputIndex + 1]
            } else if len(tokens) == outputIndex + 3 {
                fmt.Printf("Locale: %s\n", inputFile)
                if len(locale.Name) > 0 {
                    impl.addLocale(&locale)
                }
                locale.Name = tokens[outputIndex + 2]
            } else {
                fmt.Printf("skipping %s\n", inputFile)
                return nil
            }
        } else if len(locale.Name) == 0 {
                return  errors.New(fmt.Sprintf("Wrong directory structure: %s\n", inputFile))
        } else {
            //fmt.Printf("File: %s\n", inputFile)

            // Read and parse the data
            textFile := strings.Replace(inputFile, "/output/" + impl.Name, "/input", 1)
            textContent, err := ioutil.ReadFile(textFile)
            check(err)

            if len(textContent) == 0 {
                fmt.Printf("Empty text file %s\n", inputFile);
                return nil;
            }

            var actualData ParsedData
            actualData, err = parseFile(inputFile)
            if err != nil {
                return  errors.New(fmt.Sprintf("Cannot parse output file %s: %s\n", inputFile, err.Error()))
            }

            var validationData ParsedData
            validationFile := strings.Replace(inputFile, "/output/" + impl.Name, "/validation", 1)
            validationData, err = parseFile(validationFile)
            if err != nil {
                return  errors.New(fmt.Sprintf("Cannot parse validation file%s: %s\n", validationFile, err.Error()))
            }

            // Compare the data
            var delta CalculatedDelta
            delta = compareData(validationData, actualData)
            file := NewFile(tokens[len(tokens) - 1], locale.Name, string(textContent), delta)

            // Update locale statistics
            locale.Add(*file)
        }
        return nil
    })
    impl.addLocale(&locale)
    addImpl(web, &impl)
    return err
}

func main() {
    var (
        root = flag.String("root", "~/datasets", "Folder (inputs for the table expected to be under <Folder>/output/>")
        removeNoise = flag.Bool("removeNoise", true, "Removes all unsignificant differences")
    )
    flag.Parse()
    if *root == "" {
        fmt.Println("Must set --root")
        flag.PrintDefaults()
    }
    expand(root)

    // Define the data to be used in the template
    web := WebPage {
        Title: "Comparison Table (accuracy, performance and disk memory)",
    }

    // Parse the template
    t, err := template.ParseFiles("../html/index.html", "../html/scripts.html", "../html/tbody.html")

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
