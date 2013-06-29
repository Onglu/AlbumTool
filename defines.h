#ifndef DEFINES_H
#define DEFINES_H

#define MINI_WIDTH          1024
#define MINI_HEIGHT         560

#define TEMP_FILE           "/DDECF6B7F103CFC11B2.png"
#define PKG_FMT             ".xcmb"
#define PIC_FMT             ".jpg"
#define PKG_PASSWORD        "123123"

#ifdef FROM_PACKAGE
#define PKG_ENCRYPT         0
#endif

#define BACKEND_DB          "backend.sqlite"

enum layer_type{LT_Background, LT_Photo, LT_Decoration, LT_Mask};

#endif // DEFINES_H
