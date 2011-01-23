project(chartingshape)

find_package(KDE4 REQUIRED)
find_package(KOfficeLibs REQUIRED)

include (KDE4Defaults)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} ${KOFFICELIBS_INCLUDE_DIR})

add_subdirectory(doc)
add_subdirectory(chartshape)

# The following two dirs have been disabled since before the conversion to git
# add_subdirectory(filters)
# add_subdirectory(kchart)

