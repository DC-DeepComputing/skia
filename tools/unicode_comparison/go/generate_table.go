// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
  //"bufio"
  "path/filepath"
  "io/ioutil"
	"flag"
	"fmt"
  "html/template"
	//"math"
	"os"
  "strings"
  "strconv"
)

func Abs(x int) int {
    if x < 0 {
            return -x
    }
    return x
}

func check(e error) {
    if e != nil {
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

type Ratio struct {
    Num int
    Total int
}

func (this Ratio) Divide () string {
    return fmt.Sprintf("%.6f", float64(this.Num)/float64(this.Total))
}

func (this *Ratio) Add(other Ratio) {
    if other.Num > 0 {
        this.Num += other.Num
        this.Total += other.Total
    }
}

type CalculatedDelta struct {
    Memory float64
    Performance float64
    Disk float64
    Graphemes Ratio
    SoftBreaks Ratio
    HardBreaks Ratio
    Whitespaces Ratio
    Words Ratio
    Controls Ratio
}

func (this *CalculatedDelta) Clean() {
    this.Performance = 0.0
    this.Memory = 0.0
    this.Disk = 0.0
    this.Count = 0
    this.Total = 0
    this.GraphemesDiff = Ratio { 0, 1 }
    this.SoftBreaksDiff = Ratio { 0, 1 }
    this.HardBreaksDiff = Ratio { 0, 1 }
    this.WhitespacesDiff = Ratio { 0, 1 }
    this.WordsDiff = Ratio { 0, 1 }
    this.ControlsDiff = Ratio { 0, 1 }
}

func (this *CalculatedDelta) Add(other: CalculatedDelta) {
    this.Performance += other.Performance
    this.Memory += other.Memory
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

// Locale data
type Locale struct {
    Name string
    Delta CalculatedDelta
}

func NewLocale(name: string) Locale {
    return &Locale { Name: loc }
}

func (this *Locale) Clean() {
    this.Language = ""
    this.Clean()
}

func (this *Locale) Add(delta: CalculatedDelta) {
    this.Delta.add(delta)
    this.Count += 1
}

// Implementation data (with all the locales)
type Impl struct {
    Name string
    Locales [] Locale
    Delta CalculatedDelta
}

func NewImpl(name: string) Impl {
    return &Impl { Name: Impl }
}

func (this *Impl) Clean() {
    this.Name = ""
    this.Rows = nil
    this.Clean()
}

func (this *Impl) addLocale(other *Locale) {
    this.Rows = append(this.Rows, *other)
    this.Delta.add(other.Delta)
    other.Clean();
}

type WebPage struct {
    Title string
    Heading string
    Rows [] Impl
}

func addImpl(web *WebPage, impl *Impl) {
    web.Rows = append(web.Rows, *Impl)
    impl.Clean()
}

func parseFile(path string) ParsedData, error {
    // Time: float64
    // Memory: float64
    // Graphemes: n1 n2 ...
    // SoftBreaks: n1 n2 ...
    // HardBreaks: n1 n2 ...
    // Whitespaces: n1 n2 ...
    // Words: n1 n2 ...
    // Controls: n1 n2 ...
    lines := strings.Split(path, "\n")
    if (len(lines) < 8) {
        fmt.Println("Wrong data format, skipped)")
        return nil, 
    }
    var err error
    result.Time, err = strconv.ParseFloat(lines[0], 64)
    if (err != nil) {
        fmt.Println("Wrong time format, skipped)")
        return false
    }
    result.Memory, err = strconv.ParseFloat(lines[1], 64)
    if (err != nil) {
        fmt.Println("Wrong memory format, skipped)")
        return false
    }
    result.Graphemes = lines[2]
    result.SoftBreaks = lines[3]
    result.HardBreaks = lines[4]
    result.Whitespaces = lines[5]
    result.Words = lines[6]
    result.Controls = lines[7]

    return true;
}

func diff(expected []int, actual []int) []int {

    var result [] int
    result = append(result, len(expected))

    e := 1
    a := 1
    for e < len(expected) || a < len(actual) {
        if e >= len(expected) {
            result = append(result, -1 * actual[a])
            a += 1
        } else if a >= len(actual) {
            result = append(result, +1 * expected[e])
            e += 1
        } else if actual[a] < expected[e] {
            result = append(result, -1 * actual[a])
            a += 1
        } else if actual[a] > expected[e] {
            result = append(result, +1 * actual[a])
            e += 1
        } else {
            a += 1
            e += 1
        }
    }

    if len(result) > 1 {
        fmt.Printf("Diff %d:\n%s\n%s\n", len(result) - 1, expected, actual)
    }
    return result;
}

func insert(a []int, index int, value int) []int {
    if len(a) == index { // nil or empty slice or after last element
        return append(a, value)
    }
    a = append(a[:index+1], a[index:]...) // index < len(a)
    a[index] = value
    return a
}

func extend(this *[]int, other []int) bool {
    
    changed := false
    t := 1
    o := 1
    for t < len(*this) && o < len(other) {
        if other[o] < (*this)[t] {
            changed = true
            *this = insert(*this, t, other[o])
            o += 1
        } else if other[o] == (*this)[t] {
            t += 1
            o += 1
        } else {
            t += 1
        }
    }
    for o < len(other) {
        changed = true
        *this = append(*this, other[o])
        o += 1
    }
    return changed
}

func split(str string, sep string)[] int {
    arr := strings.Split(str, sep)
    var result = []int {}
    for _, i := range arr {
        j, err := strconv.Atoi(i)
        if err != nil {
            panic(err)
        }
        result = append(result, j)
    }
    return result
}

func compareData(expected Data, actual Data, removeNoise bool) Diff {
    var difference Diff

    expectedGraphemes := split(expected.Graphemes, " ")
    expectedSoftBreaks := split(expected.SoftBreaks, " ")
    expectedHardBreaks := split(expected.HardBreaks, " ")
    expectedWhitespaces := split(expected.Whitespaces, " ")
    expectedWords := split(expected.Words, " ")
    expectedControls := split(expected.Controls, " ")

    actualGraphemes := split(actual.Graphemes, " ")
    actualSoftBreaks := split(actual.SoftBreaks, " ")
    actualHardBreaks := split(actual.HardBreaks, " ")
    actualWhitespaces := split(actual.Whitespaces, " ")
    actualWords := split(actual.Words, " ")
    actualControls := split(actual.Controls, " ")
    
    if removeNoise {
        // Add hard line breaks to soft line breaks
        changed1 := extend(&expectedSoftBreaks, expectedHardBreaks)
        changed2 := extend(&actualSoftBreaks, actualHardBreaks)
        if changed1 || changed2 {
            fmt.Println("Some noise was removed\n")
        }
    }

    difference.Graphemes = diff(expectedGraphemes, actualGraphemes)
    difference.SoftBreaks = diff(expectedSoftBreaks, actualSoftBreaks)
    difference.HardBreaks = diff(expectedHardBreaks, actualHardBreaks)
    difference.Whitespaces = diff(expectedWhitespaces, actualWhitespaces)
    difference.Words = diff(expectedWords, actualWords)
    difference.Controls = diff(expectedControls, actualControls)

    return difference
}

func calculateAccuracy(line []int) Ratio {
    if len(line) == 0 {
        fmt.Println("Wrong difference formant")
        return Ratio { 0, 1 }
    }
    if len(line) > 1 {
        fmt.Printf("diff=%f\n", float64(len(line) - 1) / float64(line[0]))
        return Ratio { len(line) - 1, line[0] }
    } else {
        return Ratio { 0, 1 }
    }
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

        if info.IsDir() {
            tokens := strings.Split(inputFile, "/")
            if inputFile[0] == '.' {
                // ~/datasets/output
            } else if len(tokens) == 3 {
                fmt.Printf("Impl: %s\n", inputFile)
                impl.addLocale(&locale)
                addImpl(web, &impl)
                impl.Name = tokens[2]
            } else if len(tokens) == 4 {
                fmt.Printf("Locale: %s\n", inputFile)
                impl.addLocale(&locale)
                locale.Name = tokens[3]
            } else {
                fmt.Println("Currently only supported directory structure: <impl>/<locale>");
            }
        } else if len(locale.Name) == 0 {
                fmt.Printf("Wrong directory structure, skipped): %s\n", inputFile)
        } else {
            fmt.Printf("File: %s\n", inputFile)

            // Read and parse the data
            vat text string
            textContent, err := ioutil.ReadFile(inputFile)
            check(err)

            var actualData ParsedData
            actualData, err = parseFile(strings.Replace(inputFile, "/output/" + impl.Implementation, "/input", 1))
            if err != nil {
                fmt.Printf("Cannot parse output file: %s\n", validationFile)
                return nil
            }

            var validationData ParsedData
            validationData, err = parseFile(strings.Replace(inputFile, "/output/" + impl.Implementation, "/validation", 1))
            if err != nil {
                fmt.Printf("Cannot parse validation file: %s\n", validationFile)
                return nil
            }

            // Compare the data
            var delta CalculatedDelta
            delta, err = compareData(validationData, actualData)

            // Update locale statistics
            locale.Add(delta)
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

    div := template.FuncMap{"divide": func(a, b int) int {
        return a / b
    }}
    // Define the template
    tmpl := `
<!DOCTYPE html>
<html>
<head>
    <title>{{.Title}}</title>
</head>
<body>
    <table>
        <thead>
            <tr><th>Language</th><th>Memory</th><th>Disk</th><th>Performance (milliseconds)</th><th colspan="6">Accuracy</th></tr>
            <tr><th/><th/><th/><th>Graphemes</th><th>SoftBreaks</th><th>HardBreaks</th><th>Whitespaces</th><th>Words</th><th>Controls</th></tr>
        </thead>
        <tbody>
        {{ range $impl := .Rows }}
            <tr style="font-weight:bold">
                <td>{{ .Name }}</td>
                <td>{{ printf "%.6f" .Memory }}</td>
                <td>{{ printf "%.6f" .Disk }}</td>    
                <td>{{ printf "%.6f" .Performance }}</td>            
                <td>{{ .Graphemes.Divide }}</td>
                <td>{{ .SoftBreaks.Divide }}</td>
                <td>{{ .HardBreaks.Divide }}</td>
                <td>{{ .Whitespaces.Divide }}</td>
                <td>{{ .Words.Divide }}</td>
                <td>{{ .Controls.Divide }}</td>
            </tr>
            {{ range $locale := $impl.Rows }}
                <tr>
                    <td>&nbsp;&nbsp;&nbsp;&nbsp;{{ .Language }}</td>
                    <td>{{ printf "%.6f" .Memory }}</td>
                    <td></td>
                    <td>{{ printf "%.6f" .Performance }}</td>
                    <td>{{ .Graphemes.Divide }}</td>
                    <td>{{ .SoftBreaks.Divide }}</td>
                    <td>{{ .HardBreaks.Divide }}</td>
                    <td>{{ .Whitespaces.Divide }}</td>
                    <td>{{ .Words.Divide }}</td>
                    <td>{{ .Controls.Divide }}</td>
                </tr>
            {{ end }}
        {{ end }}
        </tbody>
    </table>
</body>
</html>`

    // Define the data to be used in the template
    web := WebPage {
        Title: "Comparison Table (accuracy, performance and disk memory)",
    }
    // Create a new template and parse the template string
    t, err := template.New("webpage").Funcs(div).Parse(tmpl)
    check(err)

    // Create index.html
    indexPath := *root + "/index.html"
    indexFile, err := os.Create(indexPath)
    check(err)

    validationPath := *root + "/validation"
    outputPath := *root + "/output"
    err = compareFiles(validationPath, outputPath, indexFile, *removeNoise, &web)
    check(err)

    // Execute the template and write the result to stdout
    err = t.Execute(indexFile, web)
    check(err)
    indexFile.Close()
}
