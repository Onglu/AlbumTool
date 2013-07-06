#ifndef DEFINES_H
#define DEFINES_H

#define MINI_WIDTH          1024
#define MINI_HEIGHT         560

#define MAX_PIC_SIZE        1920

#define MAKER_NAME          "tmaker.exe"
#define TEMP_FILE           "/DDECF6B7F103CFC11B2.png"
#define PKG_FMT             ".xcmb"
#define PIC_FMT             ".jpg"
#define PIC_NAME            "preview.png"
#define PKG_PASSWORD        "123123"

#ifdef FROM_PACKAGE
#define PKG_ENCRYPT         0
#endif

#define BACKEND_DB          "backend.sqlite"

#define LOAD_NEW            0
#define LOAD_RECORDS        1
#define LOAD_FILES          2
#define EXPORT_PAGES        3

enum ViewType{ViewType_Photo, ViewType_Template, ViewType_Album};

enum layer_type{LT_Background, LT_Photo, LT_Decoration, LT_Mask};

#endif // DEFINES_H
