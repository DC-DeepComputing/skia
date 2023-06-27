// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
  "path/filepath"
	"flag"
	"fmt"
	"github.com/trietmn/go-wiki"
	"os"
	"strconv"
	"strings"
)


func check(e error) {
    if e != nil {
        panic(e)
    }
}

func download_local_pages_by_sections(search_result []string, localInput string, start int) int {
    count := 0
    for index, element := range search_result {
        // Get the page
        page, err := gowiki.GetPage(element, -1, false, true)
        if err != nil {
            fmt.Println(err)
            continue
        }

        sections, err := page.GetSectionList()
        if err != nil {
            fmt.Println(err)
            continue
        }
        for si, section := range sections {
            if (si == len(sections) - 1) {
                // It looks like this library breaks on the last section
                break
            }
            if (len(section) == 0) {
                fmt.Println("Empty section!")
                continue
            }
            content, err := page.GetSection(section)
            if err != nil {
                fmt.Println(err)
                continue
            }

            trimmed := strings.TrimSpace(content)
            if (len(trimmed) == 0) {
                continue
            }

            localFile := localInput + "/page." + strconv.Itoa(index + start + 1) + "." + strconv.Itoa(si + 1)

            f, err := os.Create(localFile)
            check(err)
            _, err = f.WriteString(content)
            check(err)
            //fmt.Printf("%s: %v\n", localFile, len(content))
            count += 1
            defer f.Close()
        }
    }
    return count
}

func expand(path *string) {
    if strings.HasPrefix(*path, "~/") {
        home, err := os.UserHomeDir()
        check(err)
        *path = filepath.Join(home, (*path)[2:])
    }
}

func main() {
    var (
        root      = flag.String("root", "~/datasets", "Folder (pages will be downloaded under <Folder>/input")
        locale    = flag.String("locale", "*", "Locale")
        pattern   = flag.String("pattern", "*", "Pattern for search")
        fileLimit = flag.Int("files", 10, "Number of text files to download")
        pageLimit = flag.Int("pages", 5, "Number of pages to download in one attempt")
        verbose   = flag.Bool("verbose", true, "Print more details about the process")
    )
    flag.Parse()
    if *root == "" {
        fmt.Println("Must set --root")
        flag.PrintDefaults()
    }

    expand(root)
    input := *root + "/input"

    if *verbose {
        fmt.Printf("Downloading wiki pages:")
        fmt.Printf("root=%v\n", root)
        fmt.Printf("locale=%v\n", *locale)
        fmt.Printf("pattern=%v\n", *pattern)
        fmt.Printf("fileLimit=%v\n", *fileLimit)
        fmt.Printf("pageLimit=%v\n", *pageLimit)
    }

    locales := []string{ }
    if *locale != "*" {
        locales = strings.Split(*locale, ",")
    } else {
        locales = []string{"en", "ceb", "de", "sv", "fr", "nl", "ru", "es", "it", "arz", "pl", "ja", "zh", "vi", "war", "uk", "ar", "pt"}
    }

    for _, loc := range locales {
        localInput := input + "/" + loc
        err := os.MkdirAll(localInput, os.ModePerm)
        check(err)

        gowiki.SetLanguage(loc)
        fileCount := 0
        attempt := *fileLimit
        for fileCount < *fileLimit && attempt > 0 {
            files := 0
            if *pattern == "*" {
                search_result, err := gowiki.GetRandom(*pageLimit)
                check(err)
                files = download_local_pages_by_sections(search_result, localInput, fileCount)
            } else {
                search_result, _, err := gowiki.Search(*pattern, *pageLimit, true)
                check(err)
                files = download_local_pages_by_sections(search_result, localInput, fileCount)
            }
            if files == 0 {
                attempt -= 1
            }
            fileCount += files
        }
        if *verbose {
            if fileCount >= *fileLimit {
                fmt.Printf("Locale %s (%v files)\n", loc, fileCount)
            } else if fileCount == 0 {
                fmt.Printf("Locale %s does not containt text on %v attempts to download\n", loc, *fileLimit)
            } else {
                fmt.Printf("Locale %s containt less texts than %v on %v attempts to download\n", loc, *fileLimit, *fileLimit)
            }
        }
    }
}
