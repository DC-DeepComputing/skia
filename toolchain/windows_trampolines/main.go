package main

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"golang.org/x/text/encoding/charmap"
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

func transformArgs(args []string) {
	for idx, arg := range args {
		log("transform arg: %s", arg)

		arg = strings.TrimPrefix(arg, "\"")
		arg = strings.TrimSuffix(arg, "\"")

		// oldLibPrefix := `bazel-out/x64_windows-opt-exec-ST-29e576965165/bin/external/rust_windows_x86_64__x86_64-pc-windows-msvc__stable_tools/rust_toolchain\lib\rustlib\x86_64-pc-windows-msvc\lib\`
		// newLibPrefix := `external\rust_windows_x86_64__x86_64-pc-windows-msvc__stable_tools\lib\rustlib\x86_64-pc-windows-msvc\lib\`
		// if strings.HasPrefix(arg, oldLibPrefix) {
		// 	arg = strings.Replace(arg, oldLibPrefix, newLibPrefix, 1)
		// 	args[idx] = arg
		// }

		if strings.HasPrefix(arg, "-L") {
			arg = strings.Replace(arg, "-L", "/LIBPATH:", 1)
			args[idx] = arg
		}
		log("            => %s", arg)
	}
}

func callerExpectsMSVC(args []string) (bool, error) {
	for idx, arg := range args {
		if isMSVCArg(arg) {
			return true, nil
		}
		// If this argument is a paramfile (a file containing more arguments),
		// we need to read it.
		if strings.HasPrefix(arg, "@") {
			log("found param file %s", arg)
			path := strings.TrimPrefix(arg, "@")
			contents, err := os.ReadFile(path)
			if err != nil {
				return false, err
			}

			// Decode as UTF-16 if necessary.
			utf16 := unicode.UTF16(unicode.LittleEndian, unicode.ExpectBOM)
			utf16Contents, err := io.ReadAll(utf16.NewDecoder().Reader(bytes.NewReader(contents)))
			if err != nil {
				log("reading as utf-8")
			} else {
				contents = utf16Contents
				log("rewriting param file as ANSI")
				var buf bytes.Buffer
				ansi := charmap.Windows1252.NewEncoder().Writer(&buf)

				// Attempt to shorten paths.
				argsInFile := strings.Split(string(contents), "\n")
				transformArgs(argsInFile)
				contents = []byte(strings.Join(argsInFile, "\n"))
				if _, err := ansi.Write(contents); err != nil {
					return false, err
				}

				tmpDir, err := os.MkdirTemp("", "")
				if err != nil {
					return false, err
				}
				tmpFile := filepath.Join(tmpDir, "linker-params")
				if err := os.WriteFile(tmpFile, buf.Bytes(), os.ModePerm); err != nil {
					return false, err
				}
				log("Rewrote new @params: %s", tmpFile)
				args[idx] = "@" + tmpFile
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
	log(time.Now().String())
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

	transformArgs(args)

	log("Executing: " + tool + " " + strings.Join(args, " ") + "\n")
	cmd := exec.Command(tool, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		fail(err.Error())
	}
}
