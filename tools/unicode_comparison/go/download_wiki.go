// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
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

func download_local_pages_by_sections(search_result []string, localInput string, err error) {
    if err != nil {
        fmt.Println(err)
    }

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
            if (len(content) == 0) {
                continue
            }

            localFile := localInput + "/page." + strconv.Itoa(index + 1) + "." + strconv.Itoa(si + 1)

            f, err := os.Create(localFile)
            check(err)
            _, err = f.WriteString(content)
            check(err)
            defer f.Close()
        }
    }
}

func main() {
    var (
        root0     = flag.String("root", "~/datasets", "Folder (pages will be downloaded under <Folder>/input")
        locale    = flag.String("locale", "*", "Locale")
        pattern   = flag.String("pattern", "*", "Pattern for search")
        pageLimit = flag.Int("pages", 5, "Number of pages to download")
        verbose   = flag.Bool("verbose", true, "Print more details about the process")
    )
    flag.Parse()
    if *root0 == "" {
        fmt.Println("Must set --root")
        flag.PrintDefaults()
    }

    home, err :=  os.UserHomeDir()
    check(err)
    root  := strings.Replace(*root0, "~", home, 1)
    input := root + "/input"

    if *verbose {
        fmt.Printf("Downloading wiki pages:")
        fmt.Printf("root=%v\n", root)
        fmt.Printf("locale=%v\n", *locale)
        fmt.Printf("pattern=%v\n", *pattern)
        fmt.Printf("pageLimit=%v\n", *pageLimit)
    }

    locales := []string{ }
    if *locale == "*" {
        locales = strings.Split(*locale, ",")
    } else {
        locales = []string{"en", "ceb", "de", "sv", "fr", "nl", "ru", "es", "it", "arz", "pl", "ja", "zh", "vi", "war", "uk", "ar", "pt"}
    }

    for _, loc := range locales {
        localInput := input + "/" + loc
        err = os.MkdirAll(localInput, os.ModePerm)
        check(err)

        if *verbose {
            fmt.Printf("Locale %s\n", loc)
        }

        gowiki.SetLanguage(loc)
        if *pattern == "*" {
            search_result, err := gowiki.GetRandom(*pageLimit)
            download_local_pages_by_sections(search_result, localInput, err)
        } else {
            search_result, _, err := gowiki.Search(*pattern, *pageLimit, true)
            download_local_pages_by_sections(search_result, localInput, err)
        }
    }
}
