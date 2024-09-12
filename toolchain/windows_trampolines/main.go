package main

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"strings"

	"golang.org/x/text/encoding/unicode"
)

const (
	clangPath   = "external/clang_windows_amd64/bin/clang.exe"
	clangPathAr = "external/clang_windows_amd64/bin/llvm-ar.exe"
	// TODO(borenet): No hard-coded version or arch.
	msvcPathCl   = "external/clang_windows_amd64/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/cl.exe"
	msvcPathLib  = "external/clang_windows_amd64/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/lib.exe"
	msvcPathLink = "external/clang_windows_amd64/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/link.exe"
)

var logContents []string

func writeLog() {
	if err := os.WriteFile("C:\\Users\\borenet\\src\\skia\\log.txt", []byte(strings.Join(logContents, "\n")), os.ModePerm); err != nil {
		panic(err)
	}
}

func log(format string, args ...any) {
	logContents = append(logContents, fmt.Sprintf(format+"\n", args...))
}

func isMSVCArg(arg string) bool {
	return arg == "/NOLOGO"
}

func callerExpectsMSVC(args []string) (bool, error) {
	for _, arg := range args {
		if isMSVCArg(arg) {
			return true, nil
		}
		// If this argument is a paramfile (a file containing more arguments),
		// we need to read it.
		if strings.HasPrefix(arg, "@") {
			path := strings.TrimPrefix(arg, "@")
			contents, err := os.ReadFile(path)
			if err != nil {
				return false, err
			}

			// Decode as UTF-16 if necessary.
			enc := unicode.UTF16(unicode.LittleEndian, unicode.ExpectBOM)
			utf16Contents, err := io.ReadAll(enc.NewDecoder().Reader(bytes.NewReader(contents)))
			if err != nil {
				log("reading as utf-8")
			} else {
				contents = utf16Contents
			}

			// Search the args in the param file.
			for _, line := range strings.Split(string(contents), "\n") {
				line = strings.TrimSpace(line)
				line = strings.Trim(line, "\"")
				if isMSVCArg(line) {
					return true, nil
				}
			}
		}
	}
	return false, nil
}

func fail(format string, args ...any) {
	fmt.Fprintf(os.Stderr, format+"\n", args...)
	log(format, args...)
	writeLog()
	os.Exit(1)
}

func main() {
	if len(os.Args) < 2 || os.Args[1] == "--help" || os.Args[1] == "-h" {
		fail("Usage: %s <subcommand> [args...]", os.Args[0])
	}
	subcommand := os.Args[1]
	args := os.Args[2:]

	useMSVC, err := callerExpectsMSVC(args)
	if err != nil {
		fail(err.Error())
	}

	var tool string
	if useMSVC {
		log("Found flags which suggest the caller thinks they're talking to MSVC")
		switch subcommand {
		case "c_compile", "cpp_compile":
			tool = msvcPathCl
		case "link_executable":
			tool = msvcPathLink
		case "link_static_library":
			tool = msvcPathLib
		default:
			fail("Unknown subcommand %q", subcommand)
		}
	} else {
		switch subcommand {
		case "c_compile", "cpp_compile", "link_executable":
			tool = clangPath
		case "link_static_library":
			tool = clangPathAr
		default:
			fail("Unknown subcommand %q", subcommand)
		}
	}

	log("Executing: " + tool + " " + strings.Join(args, " ") + "\n")
	cmd := exec.Command(tool, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		fail(err.Error())
	}
}
