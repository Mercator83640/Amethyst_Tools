HyperFocalTester - Enhanced (Qt5)

This version adds:
- input stack list
- Z slider to browse the stack
- merged result preview
- simple depth preview
- benchmark time display
- adjustable consistency/denoise parameters

Build:
1. Open HyperFocalTester_Enhanced.pro in Qt Creator
2. Adjust OpenCV include/lib paths
3. Adjust Lib_HyperFocalTreatment include/lib paths
4. Build and run

Notes:
- Input images are expected to be 8-bit BGR
- Depth preview is a lightweight visual helper, not the library's native depth map
- Output saving can be disabled from the UI
