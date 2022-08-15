//
// Created by Maurik Holtrop on 8/15/22.
//

#ifndef CERMON_CERMONDEFINES_H
#define CERMON_CERMONDEFINES_H

#define CERMON_VERSION "1.4"
#undef  ET_DEFAULT_STATION
#define ET_DEFAULT_STATION "CERMon"

#ifndef CERMON_DEFAULT_JSON_CONFIG
#define CERMON_DEFAULT_JSON_CONFIG "CERMonConfig.json"
#endif

#ifndef CERMON_DEFAULT_JSON_CONFIG_PATH
#define CERMON_DEFAULT_JSON_CONFIG_PATH "/home/clasrun"
#endif

#ifndef DEFAULT_HISTOGRAM_PATH
#define DEFAULT_HISTOGRAM_PATH  "/home/clasrun/raster"     // This *should* have been overwritten in CMakeLists.txt
#endif

#ifndef CLI_LOGENTRY_PROGRAM
#define CLI_LOGENTRY_PROGRAM "/home/clasrun/RasterMon/logentry"
#endif

#define RED "\033[91m"
#define BLUE "\033[94m"
#define CYAN  "\033[96m"
#define GREEN  "\033[92m"
#define YELLOW "\033[93m"
#define RED "\033[91m"
#define ENDC "\033[0m"

#endif //CERMON_CERMONDEFINES_H
