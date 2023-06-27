// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package main

import (
  //"path/filepath"
	"flag"
	"fmt"
	"github.com/trietmn/go-wiki"
	"os"
	"strconv"
	"strings"
)

func download_local_pages_by_sections(search_result []string, localInput string, lastCount int, fileLimit int) int {
    countSentences := 0
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
            if len(trimmed) == 0 {
                continue
            }

            // We generate big (entire section) and small (broked by sentences)
            // texts from the same section

            sentences := GetSentences(trimmed)
            start := 0
            for i, end := range sentences {
                smallFileName := localInput + "/small/page." + strconv.Itoa(index + 1) + "." + strconv.Itoa(si + 1) + "." + strconv.Itoa( i + 1)
                smallText := strings.TrimSpace(trimmed[start:end])
                if len(smallText) == 0 {
                    continue
                }
                write_text_file(smallFileName, smallText)
                start = int(end)
                countSentences += 1

                largeFileName := localInput + "/large/page." + strconv.Itoa(index + 1) + "." + strconv.Itoa(si + 1) + "." + strconv.Itoa( i + 1)
                multiplier := 10000 / len(smallText)
                largeText := strings.Repeat(smallText, multiplier)
                write_text_file(largeFileName, largeText)

                if lastCount + countSentences >= fileLimit {
                    return countSentences
                }
            }
        }
    }
    return countSentences
}

func main() {
    var (
        root      = flag.String("root", "~/datasets", "Folder (pages will be downloaded under <Folder>/input")
        locale    = flag.String("locale", "*", "Locale")
        pattern   = flag.String("pattern", "*", "Pattern for search")
        fileLimit = flag.Int("files", 10, "Number of text files to download")
        pageLimit = flag.Int("pages", 5, "Number of pages to download in one attempt")
        textLimit = flag.Int("textLimit", 1000, "Max length of a single text")
        verbose   = flag.Bool("verbose", true, "Print more details about the process")
    )
    flag.Parse()
    if *root == "" {
        fmt.Println("Must set --root")
        flag.PrintDefaults()
    }

    if !MakeUnicode() {
        return
    }

    expand_path(root)
    input := *root + "/input"

    if *verbose {
        fmt.Printf("Downloading wiki pages:")
        fmt.Printf("root=%v\n", root)
        fmt.Printf("locale=%v\n", *locale)
        fmt.Printf("pattern=%v\n", *pattern)
        fmt.Printf("fileLimit=%v\n", *fileLimit)
        fmt.Printf("pageLimit=%v\n", *pageLimit)
        fmt.Printf("textLimit=%v\n", *textLimit)
    }

    locales := []string{ }
    if *locale != "*" {
        locales = strings.Split(*locale, ",")
    } else {
        // Sorted down by number of wiki pages
        locales = []string{"en", "ru", "it", "de", "ro", "uk", "fa", "he", "fi", "fr", "zh", "pt", "ar", "id", "tr", "th", "vi", "lv", "lt", "hr", "az", "el", "ms", "bn", "ka", "te", "ur"}
    }

    for _, loc := range locales {
        localInput := input + "/" + loc

        err := os.MkdirAll(localInput + "/large", os.ModePerm)
        check(err)

        err = os.MkdirAll(localInput + "/small", os.ModePerm)
        check(err)

        gowiki.SetLanguage(loc)
        fileCount := 0
        attempt := *fileLimit
        for fileCount < *fileLimit && attempt > 0 {
            files := 0
            if *pattern == "*" {
                search_result, err := gowiki.GetRandom(*pageLimit)
                check(err)
                files = download_local_pages_by_sections(search_result, localInput, fileCount, *fileLimit)
            } else {
                search_result, _, err := gowiki.Search(*pattern, *pageLimit, true)
                check(err)
                files = download_local_pages_by_sections(search_result, localInput, fileCount, *fileLimit)
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

    DestroyUnicode()
}
