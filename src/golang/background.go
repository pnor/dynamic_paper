package main

import (
	"C"
	"fmt"
	"github.com/xyproto/wallutils"
)

//export SetBackground
func SetBackground(fileNameString *C.char, modeString *C.char) {
	fileName := C.GoString(fileNameString)
	mode := C.GoString(modeString)

	err := wallutils.SetWallpaperCustom(fileName, mode, false)

	if err != nil {
		fmt.Printf("Error occured when setting %v as the background with mode %v\nerror:%v\n",
			fileName, mode, err)
	}
}

func main() {}
