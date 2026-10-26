#ifndef NNSR_H_
#define NNSR_H_

#define _ERROR_	-1

#define MAX_PATH_SIZE			194
#define MAX_FILENAME_SIZE		12
#define MAX_PATH_FILENAME_SIZE		200
#define MAX_TEST_IMAGE_NAME_SIZE	10 

/*Default parameter values*/
#define _DEFAULT_IMAGE_PATH_            "/home/cenriquez/Tesis/src/C/nnsr/images/castle_color/"
#define _DEFAULT_RESULT_IMAGE_PATH_     "/home/cenriquez/Tesis/src/C/nnsr/ImageResults/"
#define _DEFAULT_VERBOSE_               0
#define _DEFAULT_UPSCALE_FACTOR_        2
#define _DEFAULT_NUM_OF_IMAGES_         4
#define _DEFAULT_ZERO_TOLERANCE_BAND_   0.001
#define _DEFAULT_THINGRID_VALUE_        0.0
#define _DEFAULT_EDGE_BORDER_TYPE_      0
#define _DEFAULT_TEST_MOT_EST_VECTORS   0

/*Default parameter limits*/
#define _UPSCALE_FACTOR_MIN_LIMIT_      2
#define _UPSCALE_FACTOR_MAX_LIMIT_      16
#define _NUM_OF_IMAGES_MIN_LIMIT_       2
#define _NUM_OF_IMAGES_MAX_LIMIT_       16
#define _ZERO_TOLERANCE_BAND_MIN_LIMIT  0.000001
#define _ZERO_TOLERANCE_BAND_MAX_LIMIT  0.5
#define _THINGRID_VALUE_BAND_MIN_LIMIT  0.0
#define _THINGRID_VALUE_MIN_LIMIT       1.0
#define _THINGRID_VALUE_MAX_LIMIT       8.0

//Flags
#define	_TRUE_	 1
#define	_FALSE_	 0
     
#define YRC (65.738/256)
#define YGC (129.057/256)
#define YBC (25.064/256)
#define CBRC (37.945/256)
#define CBGC (74.494/256)
#define CBBC (112.439/256)
#define CRRC (112.439/256)
#define CRGC (94.154/256)
#define CRBC (18.285/256)

#define RYC     (255/219)
#define RCRC    (255/112)*0.701
#define GYC     (255/219)
#define GCBC    (255/112)*0.886*(0.114/0.587)
#define GCRC    (255/112)*0.701*(0.299/0.587)
#define BYC     (255/219)
#define BCBC    (255/112)*0.886

#define SIGMA	1.0
#define PI	3.14159265

#define dFILL  0.0
#define YFILL  255
#define CHFILL 128

#define _NO_EDGE_          0
#define _DUPLICATE_EDGE_   1
#define _FILLED_EDGE_      2

//Structs

//typedef struct map
//{
//    point *points; //
//} map_t;

/*BMP Header*/
/*
typedef struct {
     long filesize;
     char reserved[2];
     long headersize;
     long infoSize;
     long width;
     long depth;
     short biPlanes;
     short bits;
     long biCompression;
     long biSizeImage;
     long biXPelsPerMeter;
     long biYPelsPerMeter;
     long biClrUsed;
     long biClrImportant;
} BMPHEAD;
*/
/*BMP Header*/

typedef struct {
     int   filesize;
     short reserved;
     int headersize;
     int infoSize;
     int width;
     int depth;
     short biPlanes;
     short bits;
     int biCompression;
     int biSizeImage;
     int biXPelsPerMeter;
     int biYPelsPerMeter;
     int biClrUsed;
     int biClrImportant;
} BMPHEAD;

#endif /* NNSR_H_ */

