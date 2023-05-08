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

type implStruct struct {
    Implementation string
    Memory float64
    Performance float64
    Rows [] localeStruct
    GraphemesDiff Ratio
    SoftBreaksDiff Ratio
    HardBreaksDiff Ratio
    WhitespacesDiff Ratio
    WordsDiff Ratio
    ControlsDiff Ratio
}

type Ratio struct {
    Num int
    Total int
}

func (this Ratio) Divide () string {
    return fmt.Sprintf("%.6f", float64(this.Num)/float64(this.Total))
}

type localeStruct struct {
    Language string
    Memory float64
    Performance float64
    Count int
    GraphemesDiff Ratio
    SoftBreaksDiff Ratio
    HardBreaksDiff Ratio
    WhitespacesDiff Ratio
    WordsDiff Ratio
    ControlsDiff Ratio
}

type WebPage struct {
    Title string
    Heading string
    Rows [] implStruct
}

type Data struct {
    Time float64
    Memory float64
    Disk float64
    Graphemes string
    SoftBreaks string
    HardBreaks string
    Whitespaces string
    Words string
    Controls string
}

type Diff struct {
    Count int
    Graphemes [] int
    SoftBreaks [] int
    HardBreaks [] int
    Whitespaces [] int
    Words [] int
    Controls [] int
}

func (this *localeStruct) Clean() {
    this.Language = ""
    this.Performance = 0
    this.Memory = 0
    this.Count = 0
    this.GraphemesDiff = Ratio { 0, 1 }
    this.SoftBreaksDiff = Ratio { 0, 1 }
    this.HardBreaksDiff = Ratio { 0, 1 }
    this.WhitespacesDiff = Ratio { 0, 1 }
    this.WordsDiff = Ratio { 0, 1 }
    this.ControlsDiff = Ratio { 0, 1 }
}

func (this *Ratio) Add(other Ratio) {
    if other.Num > 0 {
        this.Num += other.Num
        this.Total += other.Total
    }
}

func (this *implStruct) Clean() {
    this.Implementation = ""
    this.Performance = 0
    this.Memory = 0
    this.Rows = nil
    this.GraphemesDiff = Ratio { 0, 1 }
    this.SoftBreaksDiff = Ratio { 0, 1 }
    this.HardBreaksDiff = Ratio { 0, 1 }
    this.WhitespacesDiff = Ratio { 0, 1 }
    this.WordsDiff = Ratio { 0, 1 }
    this.ControlsDiff = Ratio { 0, 1 }
}

func (this *implStruct) addLocale(other *localeStruct) {
    this.Performance += other.Performance
    if other.Count > 0 {
        other.Performance /= float64(other.Count)
        other.Performance /= 1000000.0
    }
    this.Memory += other.Memory
    this.Rows = append(this.Rows, *other)

    this.GraphemesDiff.Add(other.GraphemesDiff)
    this.SoftBreaksDiff.Add(other.SoftBreaksDiff)
    this.HardBreaksDiff.Add(other.HardBreaksDiff)
    this.WhitespacesDiff.Add(other.WhitespacesDiff)
    this.WordsDiff.Add(other.WordsDiff)
    this.ControlsDiff.Add(other.ControlsDiff)

    other.Clean();
}

func addImpl(web *WebPage, impl *implStruct) {
    if len(impl.Rows) > 0 {
        if len(impl.Rows) > 0 {
            impl.Performance /= float64(len(impl.Rows))
            impl.Performance /= 1000000.0
        }
        web.Rows = append(web.Rows, *impl)
        impl.Clean()
    }
}

func parseFile(path string, result *Data) bool {
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
        return false
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

    var impl implStruct
    var locale localeStruct
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
                // ./datasets/output
            } else if len(tokens) == 3 {
                impl.addLocale(&locale)
                addImpl(web, &impl)
                impl.Implementation = tokens[2]
            } else if len(tokens) == 4 {
                impl.addLocale(&locale)
                locale.Language = tokens[3]
            } else {
                fmt.Println("Currently only suspported directory structure: <impl>/<locale>");
            }
        } else if len(locale.Language) == 0 {
                fmt.Printf("Wrong directory structure, skipped): %s\n", inputFile)
                return nil
        } else {
            // Read text file
            textFile := strings.Replace(inputFile, "/output/" + impl.Implementation, "/input", 1)
            textContent, err0 := ioutil.ReadFile(textFile)
            check(err0)

            // Read validation file
            validationFile := strings.Replace(inputFile, "/output/" + impl.Implementation, "/validation", 1)
            validationContent, err1 := ioutil.ReadFile(validationFile)
            check(err1)

            // Read input file
            fileContent, err2 := ioutil.ReadFile(inputFile)
            check(err2)

            // Parse compareFiles
            var expected Data
            if !parseFile(string(validationContent), &expected) {
                fmt.Printf("Cannot parse validation file: %s\n", validationFile)
                return nil
            }

            var actual Data
            if !parseFile(string(fileContent), &actual) {
                fmt.Printf("Cannot parse input file: %s\n", inputFile)
                return nil
            }

            // Compare files
            diff := compareData(expected, actual, removeNoise)

            // Update locale statistics
            locale.Performance += actual.Time
            locale.Memory += actual.Memory
            locale.GraphemesDiff.Add(calculateAccuracy(diff.Graphemes))
            locale.SoftBreaksDiff.Add(calculateAccuracy(diff.SoftBreaks))
            locale.HardBreaksDiff.Add(calculateAccuracy(diff.HardBreaks))
            locale.WhitespacesDiff.Add( calculateAccuracy(diff.Whitespaces))
            locale.WordsDiff.Add(calculateAccuracy(diff.Words))
            locale.ControlsDiff.Add(calculateAccuracy(diff.Controls))
            locale.Count += 1

            fmt.Printf("Compare %s vs %s\n", validationFile, inputFile)
            if len(diff.Graphemes) > 1 {
                fmt.Printf("   Graphemes: %f\n", locale.GraphemesDiff)
                printDifference(string(textContent), diff.Graphemes)
            }
            if len(diff.SoftBreaks) > 1 {
                fmt.Printf("   SoftBreaks: %f\n", locale.SoftBreaksDiff)
                printDifference(string(textContent), diff.SoftBreaks)
            }
        }
        return nil
    })
    impl.addLocale(&locale)
    addImpl(web, &impl)
    return err
}

func main() {
    var (
        root = flag.String("root", "./datasets", "Folder (inputs for the table expected to be under <Folder>/output/>")
        removeNoise = flag.Bool("removeNoise", true, "Removes all unsignificant differences")
    )
    flag.Parse()
    if *root == "" {
        fmt.Println("Must set --root")
        flag.PrintDefaults()
    }

    div := template.FuncMap{"divide": func(a, b int) int {
        return a / b
    }}
    // Define the template
    /*
                <td>{{ printf "%.6f" .GraphemesDiff.Num/.GraphemesDiff.Total }}</td>
                <td>{{ printf "%.6f" .SoftBreaksDiff.Num/.SoftBreaksDiff.Total }}</td>
                <td>{{ printf "%.6f" .HardBreaksDiff.Num/.HardBreaksDiff.Total }}</td>
                <td>{{ printf "%.6f" .WhitespacesDiff.Num/.WhitespacesDiff.Total }}</td>
                <td>{{ printf "%.6f" .WordsDiff.Num/.WordsDiff.Total }}</td>
                <td>{{ printf "%.6f" .ControlsDiff.Num/.ControlsDiff.Total }}</td>
    */
    tmpl := `
<!DOCTYPE html>
<html>
<head>
    <title>{{.Title}}</title>
</head>
<body>
    <table>
        <thead>
            <tr><th>Language</th><th>Memory</th><th>Performance (milliseconds)</th><th colspan="6">Accuracy</th></tr>
            <tr><th/><th/><th/><th>Graphemes</th><th>SoftBreaks</th><th>HardBreaks</th><th>Whitespaces</th><th>Words</th><th>Controls</th></tr>
        </thead>
        <tbody>
        {{ range $impl := .Rows }}
            <tr style="font-weight:bold">
                <td>{{ .Implementation }}</td>
                <td>{{ printf "%.6f" .Memory }}</td>
                <td>{{ printf "%.6f" .Performance }}</td>
                <td>{{ .GraphemesDiff.Divide }}</td>
                <td>{{ .SoftBreaksDiff.Divide }}</td>
                <td>{{ .HardBreaksDiff.Divide }}</td>
                <td>{{ .WhitespacesDiff.Divide }}</td>
                <td>{{ .WordsDiff.Divide }}</td>
                <td>{{ .ControlsDiff.Divide }}</td>
            </tr>
            {{ range $locale := $impl.Rows }}
                <tr>
                    <td>&nbsp;&nbsp;&nbsp;&nbsp;{{ .Language }}</td>
                    <td>{{ printf "%.6f" .Memory }}</td>
                    <td>{{ printf "%.6f" .Performance }}</td>
                    <td>{{ .GraphemesDiff.Divide }}</td>
                    <td>{{ .SoftBreaksDiff.Divide }}</td>
                    <td>{{ .HardBreaksDiff.Divide }}</td>
                    <td>{{ .WhitespacesDiff.Divide }}</td>
                    <td>{{ .WordsDiff.Divide }}</td>
                    <td>{{ .ControlsDiff.Divide }}</td>
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
