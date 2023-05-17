// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
  //"bufio"
	"flag"
	"fmt"
  "html/template"
	"os"
)

func check(e error) {
    if e != nil {
        panic(e)
    }
}

type rowStruct struct {
    Language string
    Memory float32
    Performance float32
    Accuracy float32
}

type webPage struct {
    Title string
    Heading string
    Rows [] rowStruct
}

func main() {
    var (
        root = flag.String("root", "./datasets", "Folder (inputs for the table expected to be under <Folder>/output/>")
    )
    flag.Parse()
    if *root == "" {
        fmt.Println("Must set --root")
        flag.PrintDefaults()
    }
    // Define the template
    tmpl := `
<!DOCTYPE html>
<html>
<head>
    <title>{{.Title}}</title>
</head>
<body>
    <h1>{{.Heading}}</h1>
    <table>
        <thead>
            <tr><th>Language</th><th>Disk Memory</th><th>Performance</th><th>Accuracy</th></tr>
        </thead>
        <tbody>
        {{ range .Rows}}
            <tr>
                <td>{{ .Language }}</td>
                <td>{{ .Memory }}</td>
                <td>{{ .Performance }}</td>
                <td>{{ .Accuracy }}</td>
            </tr>
        {{ end}}
        </tbody>
    </table>
</body>
</html>`

    // Define the data to be used in the template
    web := webPage {
        Title: "Comparison Table (accuracy, performance and disk memory)",
        Heading: "All possible languages from Wikipedia via SkUnicode interface",
    }

    for i := 1; i < 10; i++ {
        view := rowStruct{
            Language: "en",
            Memory:   1000.0,
            Performance: 100.0,
            Accuracy:    35.5,
        }

        web.Rows = append(web.Rows, view)
    }

    // Create a new template and parse the template string
    t, err := template.New("webpage").Parse(tmpl)
    check(err)

    // Create index.html
    outputPath := *root + "/index.html"
    file, err := os.Create(outputPath)
    check(err)

    // Execute the template and write the result to stdout
    err = t.Execute(file, web)
    check(err)
    file.Close()
}
