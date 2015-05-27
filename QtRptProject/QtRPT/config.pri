CONFIG(debug, debug|release) {
    DEST_DIRECTORY = $$PWD/../bin/debug
}
CONFIG(release, debug|release) {
    DEST_DIRECTORY = $$PWD/../bin/release
}
PROJECT_ROOT_DIRECTORY = $$PWD #not $$_PRO_FILE_PWD_!

