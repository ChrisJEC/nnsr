// Standard ANSI C header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <float.h>
// External libraries header files
#include <jpeglib.h>
#include <fftw3.h>
#include <nn.h>
// Local header files	
#include "../include/nnsr.h"

// Parameter arrays
char ImagesPath[MAX_PATH_SIZE]   = ""; //Input low resolution images path
char HRImagesPath[MAX_PATH_SIZE] = ""; //Output high resolution images path

// Parameter variables
int verbose             = _DEFAULT_VERBOSE_;             //verbose level
int factor              = _DEFAULT_UPSCALE_FACTOR_;      //up-scale factor							
int NumLRimages         = _DEFAULT_NUM_OF_IMAGES_;       //Number of input low resolution images
int EdgeBorderType      = _DEFAULT_EDGE_BORDER_TYPE_;    //Set external border type [0: No border, 1: Duplicate pixel border, 2:Fill border]
int TestMEVectors       = _DEFAULT_TEST_MOT_EST_VECTORS; //Set pre-defined motion estimated vectors [1 to 5]
double ZeroTolerance    = _DEFAULT_ZERO_TOLERANCE_BAND_; //Pending       
double Thingrid         = _DEFAULT_THINGRID_VALUE_;      //Pending  

//Parameter flags
int Grayscale           = _FALSE_; //If true, generates Luminance, blue chrominance and red chrominance images
int Extrapolation       = _FALSE_; //If true, performs extrapolation for external border pixels

//Global variables
int width;              //Low resolution image width
int height;             //Low resolution image height
int LRdimension;        //Low resolution image dimension
int HRwidth;            //High resolution image width
int HRheight;           //Low resolution image height
int HRdimension;        //High resolution image dimension
int bytes_per_pixel;    //Bytes per pixel (1: Grayscale Images: 3: Color Images)
int Xo, Yo;             //Image center coordinates
int nout;               //Number of mapped points
 
// Global dynamic arrays
unsigned char *Raw_LR_Image = NULL;             //Low resolution raw image
unsigned char *Y_LR_Image   = NULL;             //Low resolution luminance component
unsigned char *Cb_LR_Image  = NULL;             //Low resolution blue chrominance component
unsigned char *Cr_LR_Image  = NULL;             //Low resolution red chrominance component
unsigned char *Y_HR_Image   = NULL;             //High resolution luminance component
unsigned char *Cb_HR_Image  = NULL;             //High resolution blue chrominance component
unsigned char *Cr_HR_Image  = NULL;             //High resolution red chrominance component
unsigned char *HR_3_Channels_Image = NULL;      //High resolution Three channel bitmap image
point *mapped_image = NULL;                     //Set of pixels mapped in the high resolution grid
point *Y_HR_Normalized_Image = NULL;            //Normalized high resolution luminance component

// Global 2-D dynamic arrays
double **Gray_Norm_LR_Image_Set = NULL;      //Y-channel normalized low resolution image set
double **motion_est             = NULL;      //Motion estimated vectors

/**********Main functions header*********/
//Read low resolution images
int Read_LR_Images(char *path, int ims_number); 
// Calculates motion estimation vectors
int Motion_Estimation(double **image_set, double **motion_estimation_vectors);
// Map pixels in the hig resolution grid
int HR_grid_map(double **Normalized_Grayscale_Image_Set, double **Motion_Vectors, point *mapped_points);
// Performs Nearest Neighbor Interpolation on mapped image
int HR_Image_NN_Interpolation(point *pin, int nin, point *pout);
//Denormalizes luminance color channel
int Denormalize_Color_Channel(point *Normalized_Image_Channel, unsigned char *Denormalized_Image_Channel, int dimension);
//Resizes one channel image
int Image_Resize(unsigned char *image, unsigned char *resized_image);
//Constructs high resolution bitmap image
int Construct_HR_Bitmap_Image(unsigned char *Y_Channel, unsigned char *Cb_Channel, unsigned char *Cr_Channel, int dimension);

/**********Secondary functions header*********/
//Reads a single JPEG file
int Read_JPEG_File(const char *filename);
void RGB_to_YCbCr(unsigned char *rgb_image, unsigned char *y_image,
		unsigned char *cb_image, unsigned char *cr_image);
void Print_Image_Values(unsigned char *vector, int w, int h);
void Print_dMatrix(double **matrix, int w, int h);
void Print_fftwVector(fftw_complex *vector, int w, int h);
void Normalize_Grayscale_Image(unsigned char *gray_image, int imagenumber, int dimension);
unsigned char *Init_ucVector(int dim);
fftw_complex *Init_fftwVector(int dim);
point *Init_pointVector(int dim);
double **Init_dMatrix(int nrows, int ncols);
int Free_Matrix(double **matrix, int nrows);
void RGB_to_Gray(unsigned char *rgb_image, unsigned char *g_image);
void complex_multiplication(fftw_complex *result, fftw_complex *A, fftw_complex *B, int dim);
void normalize_matrix(fftw_complex *A, int dim);
void calculate_R(fftw_complex *Result,fftw_complex *Ty, fftw_complex *Tx, fftw_complex *X, fftw_complex *Y, int width, int height);
double calculate_matrix_coefficient(fftw_complex *A, fftw_complex *B, int dim);
void Print_dVector(double *vector, int w, int h);
void mtxinv(double *Ai, double *A);
void mtxebesub(fftw_complex *Result, fftw_complex *A, fftw_complex *B, int dim);
void  mtxmult(double *s, double *A, double *B);
void print_pointVector (point *vector, int dim);
void print_pointMatrix(point *matrix, int sizeh, int sizew);

void Construct_Grayscale_Image(unsigned char *Y_component, unsigned char *Grayscale_Bitmap_Image);
int write_bmp_file( char *filename, unsigned char *Grayscale_Bitmap_Image);
int YCbCr_to_RGB(unsigned char *RGB_Image, unsigned char *Y_Channel, unsigned char *Cb_Channel,unsigned char *Cr_Channel);
int Duplicate_Border_Pixels(unsigned char *the_image, int border_width, int image_width, int image_height, int BorderType);
static void usage(unsigned char);

//int YCbCr_to_RGB(unsigned char *RGB_Image, unsigned char *Y_Channel, unsigned char *Cb_Channel,unsigned char *Cr_Channel);

int main(int argc, char *argv[])
{
	char c; //store arguments

	while ((c = getopt(argc, argv, "p:o:c:v:f:n:m:z:b:t:GEH")) != -1) //Parse program arguments
	{
		switch (c)
		{
		case 'p':                              //Set low resolution input images path parameter
			strcpy(ImagesPath, optarg);
			break;

		case 'o':
			strcpy(HRImagesPath, optarg); //Set high resolution output images path parameter
			break;

		case 'v':                             //Set verbose level parameter
			verbose = atoi(optarg);       
			break;

		case 'f':                             //Set hih resolution up-scale factor parameter
			factor = atoi(optarg);         
			break;

		case 'n':                             //Set number of low resolution images parameter
			NumLRimages = atoi(optarg);
			break;

		case 'm':                             //Set pre-defined motion estimated vector parameter
			TestMEVectors = atoi(optarg); 
			break;

		case 'z':                             //Set tolerance band (0±tolerance) in order to round angles close to zero.
                        ZeroTolerance = atof(optarg); 
			break;

		case 'b':                             //Set edge border type parameter
			EdgeBorderType = atoi(optarg);
			break;

		case 't':                            //Pending
			Thingrid = atof(optarg);
			break;

		case 'G':                           //Set flag to generate Y, Cr, Cb channel images
			Grayscale = _TRUE_;
			break;

		case 'E':                          //Set flag to performs an extrapolation on border pixels 
			Extrapolation = _TRUE_;
			break;

		case 'H':                          //Displays complete usage message
			usage(1);
			return (EXIT_FAILURE);

		default:                          //Display short usage message
                        usage(0);
			return (EXIT_FAILURE);
		}
	}

/************************** Parameters validation******************************/
        //Check if input low resolution images path string is empty
	if (!strcmp(ImagesPath, ""))
		sprintf(ImagesPath, _DEFAULT_IMAGE_PATH_); //If it's empty use default path

        //Check if output high resolution images path string is empty
	if (!strcmp(HRImagesPath, ""))
		sprintf(HRImagesPath, _DEFAULT_RESULT_IMAGE_PATH_); //If it's empty use default path

        //Check if up-scale factor is between upper and lower limits 
        if (factor < _UPSCALE_FACTOR_MIN_LIMIT_ || factor > _UPSCALE_FACTOR_MAX_LIMIT_)  {
		printf("The up-scale factor value must be between %d and %d\n",_UPSCALE_FACTOR_MIN_LIMIT_,_UPSCALE_FACTOR_MAX_LIMIT_), fflush(stdout);
		return (EXIT_FAILURE); 
        }

        //Check if the number of low resolution images is between upper and lower limits 
        if (NumLRimages < _NUM_OF_IMAGES_MIN_LIMIT_ || NumLRimages > _NUM_OF_IMAGES_MAX_LIMIT_)  {
		printf("The number of input images must be between %d and %d\n", _NUM_OF_IMAGES_MIN_LIMIT_, _NUM_OF_IMAGES_MAX_LIMIT_), fflush(stdout);
		return (EXIT_FAILURE);
        }

        //Check if tolerance band (to round to zero) value is between upper and lower limits 
        if (ZeroTolerance != 0.0) {
                if (ZeroTolerance < _ZERO_TOLERANCE_BAND_MIN_LIMIT || ZeroTolerance > _ZERO_TOLERANCE_BAND_MAX_LIMIT)  {
		        printf("The zero tolerance band must be between %f and %f\n", _ZERO_TOLERANCE_BAND_MIN_LIMIT, _ZERO_TOLERANCE_BAND_MAX_LIMIT), fflush(stdout);
		        return (EXIT_FAILURE);
                }
        }

        //Check if the edge border type value is 1(no border), 2(duplicate pixel border) or 3(filled border)
        if (EdgeBorderType < _NO_EDGE_ || EdgeBorderType > _FILLED_EDGE_)  {
		printf("The edge border type must be %d:No edge, %d:Duplicate edge or %d:Filled edge\n", _NO_EDGE_, _DUPLICATE_EDGE_, _FILLED_EDGE_), fflush(stdout);
		return (EXIT_FAILURE);
        }

        //Pending
        if (Thingrid != 0.0) {
                if (Thingrid < _THINGRID_VALUE_MIN_LIMIT || Thingrid > _THINGRID_VALUE_MAX_LIMIT)  {
		        printf("The thingrid value must be between %f and %f\n", _THINGRID_VALUE_MIN_LIMIT, _THINGRID_VALUE_MAX_LIMIT ), fflush(stdout);
		        return (EXIT_FAILURE);
                }
        }

/************************** Performs Super-Resolution***************************/

	// Reads low resolution images
	if (Read_LR_Images(ImagesPath, NumLRimages) == _ERROR_)
	{
		printf("Function %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(stdout);
		return (EXIT_FAILURE);
	}

	// Calculates motion estimation vectors
	motion_est = Init_dMatrix(NumLRimages, 3); //initializes motion estimation vectors matrix
	if (Motion_Estimation(Gray_Norm_LR_Image_Set, motion_est) == _ERROR_)
	{
		printf("Function %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(stdout);
		return (EXIT_FAILURE);
	}

	// Map pixels in the hig resolution grid
	mapped_image = Init_pointVector(NumLRimages*LRdimension); //Initializes array of pixels mapped in the high resolution grid
	if (HR_grid_map(Gray_Norm_LR_Image_Set, motion_est, mapped_image) == _ERROR_)
	{
		printf("Function %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(stdout);
		return (EXIT_FAILURE);
	} 

	// Performs Natural Neighbor Interpolation on mapped high resolution grid
	Y_HR_Normalized_Image = Init_pointVector(factor*factor*LRdimension); //Initializes normalized high resolution luminance component
	if (HR_Image_NN_Interpolation(mapped_image, nout, Y_HR_Normalized_Image) == _ERROR_)
	{
		printf("Function %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(stdout);
		return (EXIT_FAILURE);
	} 

        // Denormalizes luminance color channel
        Y_HR_Image = Init_ucVector(HRdimension); //Initializes YcbCr channels HR images
	if ( Denormalize_Color_Channel(Y_HR_Normalized_Image,  Y_HR_Image, HRdimension) == _ERROR_)
	{
		printf("Function %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(stdout);
		return (EXIT_FAILURE);
	} 
	free(Y_HR_Normalized_Image); //Normalized HR luminance component no longer needed

	// Rezises blue chrominance component image
	Cb_HR_Image = Init_ucVector(HRdimension); //Initialize blue chrominance HR image
	if (Image_Resize(Cb_LR_Image, Cb_HR_Image) == _ERROR_)
	{
		printf("Function %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(stdout);
		return (EXIT_FAILURE);
	} 
        free(Cb_LR_Image); //LR blue chrominance component no longer needed
        
	// Rezises red chrominance component image
	Cr_HR_Image = Init_ucVector(HRdimension); //Initialize red chrominance HR image
	if (Image_Resize (Cr_LR_Image, Cr_HR_Image) == _ERROR_)
	{
		printf("Funcion %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(stdout);
		return (EXIT_FAILURE);
	} 
        free(Cr_LR_Image); //LR red chrominance component no longer needed

        //Constructs high resolution bitmap image
	if (Construct_HR_Bitmap_Image(Y_HR_Image, Cb_HR_Image, Cr_HR_Image, HRdimension) == _ERROR_)
	{
		printf("Funcion %s <%d>: 	nnsr stopped\n", __FUNCTION__, __LINE__), fflush(
				stdout);
		return (EXIT_FAILURE);
	} 

        //Free HR YCbCr components 
        free(Y_HR_Image);
        free(Cb_HR_Image);
        free(Cr_HR_Image);
        
        //Free dynamic matrices 
	Free_Matrix(motion_est, NumLRimages); 
	Free_Matrix(Gray_Norm_LR_Image_Set, NumLRimages);

	return (EXIT_SUCCESS);
}

// Reads low resolution input images.
// Returns _TRUE_ if all the low resolution images were read succesfully.
// Inputs:
//  *path                 -- Pointer to container folder path of LR images
//  ims_number            -- Number of low resolution images to read
// Outputs:
//  *Normalized_Image_Set -- Pointer to  the normalized low resolution image 
//                           set (luminance channel). Function already initializes 
//                           dynamic array. It is necessary to free dynamic memory
//                           ater   
int Read_LR_Images(char *path, int ims_number)
{
	char ImageFilenamePath[MAX_PATH_FILENAME_SIZE]; //Image path+filename
	char ImageFileName[MAX_FILENAME_SIZE]; //Image filename

	int ImNumIndex; //Image number index

	//Build first LR image filename
	strcpy(ImageFilenamePath, path);
	strcat(ImageFilenamePath, "1.jpg");

	//Read first low resolution image
	if (Read_JPEG_File(ImageFilenamePath) == _ERROR_)
		return (_ERROR_);

	//Set image parameters
	LRdimension = width * height;
	HRwidth = factor * width;
	HRheight = factor * height;
        HRdimension = HRwidth*HRheight;
        
	//Initialize YCbCr component arrays
	Y_LR_Image = Init_ucVector(LRdimension);
	Cb_LR_Image = Init_ucVector(LRdimension);
	Cr_LR_Image = Init_ucVector(LRdimension);

	//Convert from raw RGB color scheme to YCbCr color components
	RGB_to_YCbCr(Raw_LR_Image, Y_LR_Image, Cb_LR_Image, Cr_LR_Image);

	//Initialize 
	Gray_Norm_LR_Image_Set = Init_dMatrix(ims_number, LRdimension);

	//Normalizes Y-Component  and stores it  in LR image set
	Normalize_Grayscale_Image(Y_LR_Image, 0, LRdimension);

	//Read the following LR images
	for (ImNumIndex = 2; ImNumIndex <= ims_number; ImNumIndex++)
	{
		//Build ImNumIndex-th LR image filename
		strcpy(ImageFilenamePath, ImagesPath);
		sprintf(ImageFileName, "%u.jpg", ImNumIndex);
		strcat(ImageFilenamePath, ImageFileName);

		//Read ImNumIndex-th low resolution image
		if (Read_JPEG_File(ImageFilenamePath) == _ERROR_)
			return (_ERROR_);

		//Convert from raw RGB color scheme to grayscale color scheme
		RGB_to_Gray(Raw_LR_Image, Y_LR_Image);
		//Normalizes grayscale image  and stores it  in LR image set
		Normalize_Grayscale_Image(Y_LR_Image, (ImNumIndex - 1), LRdimension);
	}

	free(Y_LR_Image); //free array

	//Debug information
	if (verbose == 3)
		Print_dMatrix(Gray_Norm_LR_Image_Set, LRdimension, ims_number);

	return (_TRUE_);
}

int Motion_Estimation(double **image_set, double **motion_estimation_vectors)
{

	//local vars
	int i, j; //index
	int imnr; //Image number index
	int coord; //Vector index
	//Static Matrices
	double A[9]; //matrix of coefficients
	double Ainv[9]; //inverse of matrix of coefficients
	double b_k[3]; //constant term vectors
	double m_k[3]; //motion estimation vectors

	//Dynamic vector arrays
	fftw_complex *h; //Gaussian function
	fftw_complex *dhx; //Gaussian function partial derivative of x
	fftw_complex *dhy; //Gaussian function partial derivative of y
	fftw_complex *f_prime; //Original reference image
	fftw_complex *FFT2_h; //FFT of h(x,y);
	fftw_complex *FFT2_dhx; //FFT of dh(x,y)/dx;
	fftw_complex *FFT2_dhy; //FFT of dh(x,y)/dy;
	fftw_complex *FFT2_fprime; //FFT of f'(x,y);
	fftw_complex *T1; //f(x,y)
	fftw_complex *T2; //df(x,y)/dx
	fftw_complex *T3; //df(x,y)/dy
	fftw_complex *x_minus_Xo; //(x-Xo)
	fftw_complex *y_minus_Yo; //(y-Yo)
	fftw_complex *R; //R
	fftw_complex *g_k_prime; // g_k'(x,y)
//	fftw_complex *temp_g_k_prime; 
//	fftw_complex *temp2_g_k_prime; 
	fftw_complex *FFT2_g_k_prime; // FFT of g_k'(x,y)
	fftw_complex *g_k; // g_k(x,y)
	fftw_complex *temporal_vector = NULL; //temporary vector to store fftw_complex function results.
	fftw_complex *Ck; // C(x,y) = g_k(x,y) - f(x,y)

	//FFTW plans
	fftw_plan  plan_forward, plan_backward;

	//Initilize vector arrays
	h = Init_fftwVector(LRdimension); //Initialize Gaussian function
	dhx = Init_fftwVector(LRdimension); //Initialize Gaussian function partial derivative of x
	dhy = Init_fftwVector(LRdimension); //Initialize Gaussian function partial derivative of y
	f_prime = Init_fftwVector(LRdimension); //Initialize original reference image
	x_minus_Xo = Init_fftwVector(LRdimension); //Initialize Gaussian function partial derivative of y
	y_minus_Yo = Init_fftwVector(LRdimension); //Initialize original reference image

	/***1.Calculate Gaussian function h(x,y)***/
	/***2.Calculate Gaussian partial derivatives dh(x,y)/dx dh(x,y)/dy***/
	//Calcualte image center coordinates
	Xo = width / 2; //checar para que sirva para impares
	Yo = height / 2; //checar para que sirva para impares
	//Calculate Gaussian function and Gaussian partial deriavtives
	for (i = 1; i <= height; i++) //filas
	{
		for (j = 1; j <= width; j++) //columnas
		{
                            coord = (width*(i-1))+(j-1); //calculate vector coordinate
                            dhy[coord][0]=(-exp(-(((i-Yo)*(i-Yo))+((j-Xo)*(j-Xo)))/(2*SIGMA*SIGMA))*(i-Yo))/(2*PI*SIGMA*SIGMA); //h(x,y) real part
                            dhy[coord][1] = 0; //h(x,y) imaginary part
                            dhx[coord][0]=(-exp(-(((i-Yo)*(i-Yo))+((j-Xo)*(j-Xo)))/(2*SIGMA*SIGMA))*(j-Xo))/(2*PI*SIGMA*SIGMA); //dh(x,y)/dx real part 
                            dhx[coord][1] = 0; //dh(x,y)/dx imaginary part 
                            h[coord][0]=(exp(-(((i-Yo)*(i-Yo))+((j-Xo)*(j-Xo)))/(2*SIGMA*SIGMA)))/(2*PI*SIGMA*SIGMA); //dh(x,y)/dy real part
                            h[coord][1]=0; //dh(x,y)/dy imaginary part 
                            f_prime[coord][0] =  image_set[0][coord]; //Original reference image real part
                            f_prime[coord][1] =  0; //Original reference image imaginary part
			    x_minus_Xo[coord][0] = (j-1) - Xo;  
			    y_minus_Yo[coord][0] = (i-1)- Yo;	
			    x_minus_Xo[coord][1] = 0;  
			    y_minus_Yo[coord][1] = 0;
		}
	}

	
	if (verbose == 4) 
	{
		Print_fftwVector(f_prime,width,height);
	}


	//debug information
	if (verbose == 5) 
	{
		Print_fftwVector(dhy,width,height);
		Print_fftwVector(dhx,width,height); 
		Print_fftwVector(h,width,height);
	}

	/***3.Calculate FFTs of f'(x,y), h(x,y), dh(x,y)/dx, dh(x,y)/dy ***/
	

	FFT2_h = h; //It uses the same memory direction in order to save memory.
    	plan_forward  = fftw_plan_dft_2d( width, height , h, FFT2_h, FFTW_FORWARD, FFTW_ESTIMATE ); // FFT2_h <- FFT2(h)
    	fftw_execute( plan_forward );

	FFT2_dhx = dhx; //It uses the same memory direction in order to save memory.
    	plan_forward  = fftw_plan_dft_2d( width, height , dhx, FFT2_dhx, FFTW_FORWARD, FFTW_ESTIMATE );  // FFT2_dhx <- FFT2(dhx)
    	fftw_execute( plan_forward );

	FFT2_dhy = dhy; //It uses the same memory direction in order to save memory.
    	plan_forward  = fftw_plan_dft_2d( width, height , dhy, FFT2_dhy, FFTW_FORWARD, FFTW_ESTIMATE );  // FFT2_dhy <- fft2(dhy)
    	fftw_execute( plan_forward );

	//FFT2_fprime = Init_fftwVector(LRdimension); //Initialize original reference image

	FFT2_fprime = f_prime; //It uses the same memory direction in order to save memory.
    	plan_forward  = fftw_plan_dft_2d( width, height , f_prime, FFT2_fprime, FFTW_FORWARD, FFTW_ESTIMATE );  //FFT2_fprime <- fft2(f_prime)
    	fftw_execute( plan_forward );

	if (verbose == 6) 
	{
		Print_fftwVector(FFT2_dhy,width,height); 
		Print_fftwVector(FFT2_dhx,width,height);
		Print_fftwVector(FFT2_h,width,height);
		Print_fftwVector(FFT2_fprime,width,height); 
	}

	/***4.Calculate Terms T1 =f(x,y), T2= df(x,y)/dx, T3 = T2= df(x,y)/dy  ***/
	temporal_vector = Init_fftwVector(LRdimension); //Initialize temporal vector to store fftw_complex function results.
	//Calculate T1
	complex_multiplication(temporal_vector, FFT2_fprime, FFT2_h, LRdimension); //temporal <- FFT2(f'(x,y)).* FFT2(h(x,y))
	T1 = Init_fftwVector(LRdimension); //Requires a new array allocation because FFT2_h is used later.
        plan_backward = fftw_plan_dft_2d( width, height , temporal_vector, T1, FFTW_BACKWARD, FFTW_ESTIMATE ); //T1 <- IFFT2(FFT2(f'(x,y)).*FFT2(h(x,y)))
        fftw_execute( plan_backward );
	normalize_matrix(T1,LRdimension);
	
	//Calculate T2
	complex_multiplication(temporal_vector, FFT2_fprime, FFT2_dhx, LRdimension); //temporal <- FFT2(f'(x,y)).* FFT2(dh(x,y)/dx)
	T2 = FFT2_dhx;  //It uses the same memory sector in order to save memory.
        plan_backward = fftw_plan_dft_2d( width, height , temporal_vector, T2, FFTW_BACKWARD, FFTW_ESTIMATE ); //T2 <- IFFT2(FFT2(f'(x,y)).*FFT2(dh(x,y)/dx))
        fftw_execute( plan_backward );
	normalize_matrix(T2,LRdimension);
	//Calculate T3
	complex_multiplication(temporal_vector, FFT2_fprime, FFT2_dhy, LRdimension); //temporal <- FFT2(f'(x,y)).* FFT2(dh(x,y)/dy)
	T3 = FFT2_dhy;  //It uses the same memory sector in order to save memory.
        plan_backward = fftw_plan_dft_2d( width, height , temporal_vector, T3, FFTW_BACKWARD, FFTW_ESTIMATE ); //T3 <- IFFT2(FFT2(f'(x,y)).*FFT2(dh(x,y)/dy))
        fftw_execute( plan_backward );
	normalize_matrix(T3,LRdimension);

	if (verbose == 7) 
	{
		Print_fftwVector(T1,width,height);
		Print_fftwVector(T2,width,height);
		Print_fftwVector(T3,width,height); 
	}

	/***5.Calculate R=(df(x,y)/dy)(x-Xo)- (df(x,y)/dx)(y-Yo)   ***/
	R = FFT2_fprime; //It uses the same memory sector in order to save memory.
	calculate_R(R,T3, T2, x_minus_Xo, y_minus_Yo, width, height); 
	fftw_free(x_minus_Xo);
	fftw_free(y_minus_Yo);

	if (verbose == 8) 
	{
		Print_fftwVector(R,width,height);
	}

	//calculate_matrix_coefficient(T2,T2,LRdimension);
	/***6.Calculate Matrix A  ***/
        A[0] = calculate_matrix_coefficient(T2,T2,LRdimension);  A[1] = calculate_matrix_coefficient(T2,T3,LRdimension); A[2] = calculate_matrix_coefficient(R,T2,LRdimension);
        A[3] = calculate_matrix_coefficient(T2,T3,LRdimension);  A[4] = calculate_matrix_coefficient(T3,T3,LRdimension); A[5] = calculate_matrix_coefficient(R,T3,LRdimension);
        A[6] = calculate_matrix_coefficient(R,T2,LRdimension);   A[7] = calculate_matrix_coefficient(R,T3,LRdimension);  A[8] = calculate_matrix_coefficient(R,R,LRdimension);

	if (verbose == 9) 
	{
		Print_dVector(A,3,3);
	}

	/***7.Calculate Inverse Matrix A_inv  ***/
	mtxinv(Ainv,A);
	if (verbose == 9) 
	{
		Print_dVector(Ainv,3,3);
	}

	/***8.for each image  ***/

	//Initialize next image;
	g_k_prime = Init_fftwVector(LRdimension);
//	temp_g_k_prime = Init_fftwVector(LRdimension);
//	temp2_g_k_prime = Init_fftwVector(LRdimension);

//	FFT2_g_k_prime = Init_fftwVector(LRdimension);
	
	//Motion parameters for reference image f'(x,y) equal to zero
        motion_estimation_vectors[0][0] = 0;
        motion_estimation_vectors[0][1] = 0;
        motion_estimation_vectors[0][2] = 0;

        for (imnr=1;imnr<NumLRimages;imnr++) 
	{
	        /*save images in array*/
                for (i=0; i<height ;i++) {
                        for (j=0; j<width ;j++) {
                                coord = (width*i)+j;
                                g_k_prime[coord][0] =  image_set[imnr][coord]; 
                                g_k_prime[coord][1] =  0;
                        }
                }
	
		if (verbose == 10) 
		{
			Print_fftwVector(g_k_prime,width,height);
		}


		/***.Calculate g_k(x,y)***/	
		//Calculate FFT of g_k'
		FFT2_g_k_prime = g_k_prime; //It uses the same memory direction in order to save memory.
	    	plan_forward  = fftw_plan_dft_2d( width, height , g_k_prime, FFT2_g_k_prime, FFTW_FORWARD, FFTW_ESTIMATE ); // FFT2_g_k_prime <- FFT2(g'_k)
	    	fftw_execute( plan_forward );

		if (verbose == 11) 
		{
			Print_fftwVector(FFT2_g_k_prime,width,height);
		}

		//Calculate g_k
	 	complex_multiplication(temporal_vector, FFT2_g_k_prime, FFT2_h, LRdimension); //temporal <- FFT2(g_k'(x,y)).* FFT2(h(x,y))

		if (verbose == 12) 
		{
			Print_fftwVector(temporal_vector,width,height);
		}

		g_k = FFT2_g_k_prime; 
		plan_backward = fftw_plan_dft_2d( width, height , temporal_vector, g_k, FFTW_BACKWARD, FFTW_ESTIMATE ); //g_k <- IFFT2(FFT2(g_k'(x,y)).*FFT2(h(x,y)))
		fftw_execute( plan_backward );
		normalize_matrix(g_k,LRdimension);

		if (verbose == 13) 
		{
			Print_fftwVector(g_k,width,height);
		}

		/***9.Calculate g_k(x,y) - f(x,y) ***/
		Ck = temporal_vector;  //It uses the same memory direction in order to save memory.
		mtxebesub(Ck,g_k,T1,LRdimension);

		/***10.calculate constant term vector ***/
		b_k[0] = calculate_matrix_coefficient(T2,Ck,LRdimension);  
		b_k[1] = calculate_matrix_coefficient(T3,Ck,LRdimension); 
		b_k[2] = calculate_matrix_coefficient(R,Ck,LRdimension);

		if (verbose == 14) 
		{
			Print_dVector(b_k,1,3);
		}

		/***1.Find motion estimation vectors ***/
		mtxmult(m_k,Ainv,b_k); //A<-Ainv: s=b*Ainv

		if (verbose == 15) 
		{	
			Print_dVector(m_k,1,3);
		}

		/*****Correction Factor********/				


                if (ZeroTolerance > 0.0) {

                        double abs_angle;

                        abs_angle = m_k[2];  
                        
                        if (abs_angle < 0)
                                abs_angle = -abs_angle;
                        
                        if (abs_angle < ZeroTolerance) {
                                m_k[2] = 0;
                        }                  
       
                }

		motion_estimation_vectors[imnr][0] = m_k[0];
		motion_estimation_vectors[imnr][1] = m_k[1];
		motion_estimation_vectors[imnr][2] = m_k[2];
	        
	
	} //for (imnr=1;imnr<NumLRimages;imnr++) 

	fftw_destroy_plan( plan_forward );
        fftw_destroy_plan( plan_backward );
//	fftw_free(f_prime); //free array
	fftw_free(FFT2_h); //free array
	fftw_free(T2); //free array
	fftw_free(T3); //free array
	fftw_free(R); //free array
	fftw_free(Ck); //free array
	fftw_free(T1); //free array
	fftw_free(g_k);
//	fftw_free(g_k_prime);
//	fftw_free(temp_g_k_prime);
//	fftw_free(temp2_g_k_prime);

	if (verbose == 16) 
	{	
		Print_dMatrix(motion_estimation_vectors,3,NumLRimages);
	}


	return (_TRUE_);
}

//int HR_grid_map(double **Normalized_Grayscale_Image_Set, double **Motion_Vectors, point* mapped_points, int points_number)
int HR_grid_map(double **Normalized_Grayscale_Image_Set, double **Motion_Vectors, point *mapped_points)
{
	int x,y,k; //index
	int mp_counter; //mapped points counter
	double cos_theta_k, sin_theta_k; 

	double x_minus_Xo, y_minus_Yo;
	double a_k,b_k; //Horizontal and vertical displacements
	double X; //High resolution horizontal coordinate
	double Y; //High resolution vertical coordinate
	double Z; //HR Image intensity

        if (TestMEVectors != 0) {
	        switch (TestMEVectors) 
                { 
                case 1:
                        //Lena Color 256
                        Motion_Vectors[0][0] = 0.0000000000; Motion_Vectors[0][1] = 0.0000000000; Motion_Vectors[0][2] = 0.0000000000; 
        		Motion_Vectors[1][0] = -0.2034401133; Motion_Vectors[1][1] = 0.2032605090; Motion_Vectors[1][2] = 0.0037836780; 
                        Motion_Vectors[2][0] = -0.2314247595; Motion_Vectors[2][1] = 0.4040065373; Motion_Vectors[2][2] = 0.0093114830; 
        	        Motion_Vectors[3][0] = 0.0947696444; Motion_Vectors[3][1] = -0.2193560151; Motion_Vectors[3][2] = 0.0110294684; 
                        break;
                	        
                case 2: 
                        //Castle Color
                        Motion_Vectors[0][0] = 0.0000000000; Motion_Vectors[0][1] = 0.0000000000; Motion_Vectors[0][2] = 0.0000000000; 
                        Motion_Vectors[1][0] = -0.6265372621; Motion_Vectors[1][1] = -0.3776989528; Motion_Vectors[1][2] = 0.0003923465; 
                        Motion_Vectors[2][0] = -0.6954645447; Motion_Vectors[2][1] = -0.4800208394; Motion_Vectors[2][2] = -0.0001898431; 
                        Motion_Vectors[3][0] = -0.1307239265; Motion_Vectors[3][1] = -0.6165783596; Motion_Vectors[3][2] = -0.0001952076; 
                        break;

                case 3: 
                        //Lena Color 64         
                        Motion_Vectors[0][0] = 0.0000000000; Motion_Vectors[0][1] = 0.0000000000; Motion_Vectors[0][2] = 0.0000000000; 
                        Motion_Vectors[1][0] = 0.2105767626; Motion_Vectors[1][1] = 0.4660101021; Motion_Vectors[1][2] = 0.0009542800; 
                        Motion_Vectors[2][0] = 0.1195804277; Motion_Vectors[2][1] = 0.1137216174; Motion_Vectors[2][2] = -0.0000730607; 
                        Motion_Vectors[3][0] = 0.4141599714; Motion_Vectors[3][1] = 0.1372435544; Motion_Vectors[3][2] = 0.0003051355; 
                        break;

                case 4: 
                         //Mandril      
		        Motion_Vectors[0][0] = 0.0000000000; Motion_Vectors[0][1] = 0.0000000000; Motion_Vectors[0][2] = 0.0000000000;
		        Motion_Vectors[1][0] = 0.2102616428; Motion_Vectors[1][1] = 0.1540737681; Motion_Vectors[1][2] = 0.0057208444; 
		        Motion_Vectors[2][0] = 0.2931857628; Motion_Vectors[2][1] = 0.1832176166; Motion_Vectors[2][2] = 0.0086184395; 
		        Motion_Vectors[3][0] = 0.2919013096; Motion_Vectors[3][1] = 0.1745108542; Motion_Vectors[3][2] = 0.0084361489; 		
                        break;

                case 5 :
                        //Peppers   
		        Motion_Vectors[0][0] = 0.0000000000; Motion_Vectors[0][1] = 0.0000000000; Motion_Vectors[0][2] = 0.0000000000; 
		        Motion_Vectors[1][0] = -0.1826096986; Motion_Vectors[1][1] = -0.0179681844; Motion_Vectors[1][2] = -0.0137844034; 
		        Motion_Vectors[2][0] = 0.0599836591; Motion_Vectors[2][1] = -0.0785611026; Motion_Vectors[2][2] = -0.0166331885; 
		        Motion_Vectors[3][0] = -0.2276771659; Motion_Vectors[3][1] = -0.1448791255; Motion_Vectors[3][2] = 0.0108993089; 
                        break;
                default:
                        printf("Invalid input motion estimator vectors. Estimated vectors will be used\n"); fflush(stdout);
                        break;
                }
        }

        if (verbose==17) 
	        Print_dMatrix(Motion_Vectors,3,NumLRimages);
	
        mp_counter = 0;

	//for each Low Resolution image
	for (k=0 ;k<NumLRimages;k++) {
		//Calculate Cosines
		cos_theta_k = cos(Motion_Vectors[k][2]);
		//Calculate Sinus
		sin_theta_k  = sin(Motion_Vectors[k][2]);
		//Calculate Fa_k and Fb_k terms

		a_k = Motion_Vectors[k][0];
		b_k = Motion_Vectors[k][1];

		//Calculate coordinates
		for (y=0;y<height;y++) {
		        for (x=0;x<width;x++) {
				
				x_minus_Xo = x - Xo;
				y_minus_Yo = y - Yo;
				X = factor * ((x_minus_Xo*cos_theta_k) - (y_minus_Yo*sin_theta_k) + Xo + a_k);
				Y = factor * ((y_minus_Yo*cos_theta_k) + (x_minus_Xo*sin_theta_k) + Yo + b_k);
                       		Z = Normalized_Grayscale_Image_Set[k][(width*y) + x];
                                //Limit coordinates to the GRID
                                if ( (X>= 0) && (X <= HRwidth-1)  ) {
                                        if ( (Y >= 0) && Y <= (HRheight-1)) {
                                    	        mapped_points[mp_counter].x = X;
                                                mapped_points[mp_counter].y = Y;
                                                mapped_points[mp_counter].z = Z;
                                                mp_counter++;
                            
                                        }
                            }
			}
		}
	} 


	nout = mp_counter;

	if (verbose == 18)
		   print_pointVector(mapped_points ,mp_counter);
	return (_TRUE_);
}
        
int HR_Image_NN_Interpolation(point *pin, int nin, point *pout)
{
        int i,j; //index	
        int coord; //calculated coordinate 
        int nout = 0; //number of output points	
        double wmin;
    
        //Fill coordinates
        for (i=0;i<HRheight;i++) {
    	        for (j=0; j<HRwidth ; j++) {
                        coord = (HRwidth*i) + j;
                        pout[coord].x = j;
                        pout[coord].y = i;
                        nout++;
                }
         }

        if (Thingrid != 0.0) 
                points_thingrid(&nin, &pin, Thingrid*width, Thingrid*height);     
  

        if (Extrapolation) 
                wmin = -DBL_MAX;
        else 
                wmin = 0;
        
        nnpi_interpolate_points(nin, pin, wmin, nout, pout);

        if (verbose == 19)
        print_pointMatrix(pout, HRheight, HRwidth); 

        free(pin);

        return _TRUE_;

}

int Read_JPEG_File(const char *filename)
{
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];

	FILE *infile = fopen(filename, "rb"); //Abre archivo especificado por filename
	unsigned long location = 0;
	unsigned int i = 0;

	if (!infile)
	{
		printf("Function %s <%d>: Error opening jpeg file %s\n!", __FUNCTION__,
				__LINE__, filename), fflush(stdout);
		return (_ERROR_);
	}
	/* here we set up the standard libjpeg error handler */
	cinfo.err = jpeg_std_error(&jerr);
	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress( &cinfo);
	/* this makes the library read from infile */
	jpeg_stdio_src(&cinfo, infile);
	/* reading the image header which contains image information */
	jpeg_read_header(&cinfo, TRUE);

	width = cinfo.image_width;
	height = cinfo.image_height;
	bytes_per_pixel = cinfo.num_components;

	if (verbose == 1)
	{
		printf("JPEG File Information: \n");
		printf("File: %s\n", filename);
		printf("Image width and height: %d pixels and %d pixels.\n", width,
				height);
		printf("Color components per pixel: %d.\n", bytes_per_pixel);
		printf("Color space: %d.\n", cinfo.jpeg_color_space);
	}
	/* Start decompression jpeg here */
	jpeg_start_decompress(&cinfo);

	/* allocate memory to hold the uncompressed image */
	Raw_LR_Image = (unsigned char*) malloc(
			cinfo.output_width * cinfo.output_height * cinfo.num_components);
	/* now actually read the jpeg into the raw buffer */
	row_pointer[0] = (unsigned char *) malloc(
			cinfo.output_width * cinfo.num_components);
	/* read one scan line at a time */

// jpeg_read_scanlines( &cinfo, row_pointer, cinfo.image_height ); //del
	while (cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
			Raw_LR_Image[location++] = row_pointer[0][i];
	}
	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);
	fclose(infile);
	/* yup, we succeeded! */
	return (_TRUE_);
}

unsigned char *Init_ucVector(int dim)
{
	unsigned char *image_vector;
	image_vector = (unsigned char *) malloc(dim * sizeof(unsigned char));
	if (image_vector == NULL)
	{
		printf("Error allocating image vector in memory \n");
		exit(EXIT_FAILURE);
	}
	return image_vector;
}

fftw_complex *Init_fftwVector(int dim)
{
	fftw_complex *fftw_vector;
	fftw_vector = (fftw_complex*) fftw_malloc(dim * sizeof(fftw_complex));
	if (fftw_vector == NULL)
	{
		printf("Error allocating fftw vector in memory \n");
		exit(EXIT_FAILURE);
	}
	return fftw_vector;
}

point *Init_pointVector(int dim)
{
	point *point_vector;
	point_vector = (point *) malloc(dim * sizeof(point));
	if (point_vector == NULL)
	{
		printf("Error allocating point vector in memory \n");
		exit(EXIT_FAILURE);
	}
	return point_vector;
}

double **Init_dMatrix(int nrows, int ncols)
{
	int i;
	double **matrix;
	matrix = (double **)malloc(nrows * sizeof(double *)); 
        for (i=0; i<nrows; i++)
           matrix[i] = (double *)malloc (ncols*sizeof(double));
        
        return matrix;

}

double **init_dmatrix (int nrows,int ncols) {
        double **m;
        m = (double **)malloc (nrows * sizeof(double *));
        int i;
        for (i=0; i<nrows; i++)
           m[i] = (double *)malloc (ncols*sizeof(double));
   return m;

}


int Free_Matrix(double **matrix, int nrows)
{
	int i;
	for (i = 0; i < nrows; i++)
		free(matrix[i]);
	free(matrix);
	return (_TRUE_);
}

void RGB_to_YCbCr(unsigned char *rgb_image, unsigned char *y_image,
		unsigned char *cb_image, unsigned char *cr_image)

{
	int j;
	int index1, index2, index3;
	for (j = 0; j < height * width; j++)
	{
		index1 = 3 * j;
		index2 = 3 * j + 1;
		index3 = 3 * j + 2;

		y_image[j] = 16 + (YRC * rgb_image[index1]) + (YGC * rgb_image[index2])
				+ (YBC * rgb_image[3 * j + 2]) + 0.5;
		cb_image[j] = 128 - (CBRC * rgb_image[index1])
				- (CBGC * rgb_image[index2]) + (CBBC * rgb_image[index3]) + 0.5;
		cr_image[j] = 128 + (CRRC * rgb_image[index1])
				- (CRGC * rgb_image[index2]) - (CRBC * rgb_image[index3]) + 0.5;
	}

	//debug information
	if (verbose == 2)
	{
		printf("Y LR Image1[%dx%d]\n", width, height);
		Print_Image_Values(y_image, width, height);
		printf("Cb LR image1[%dx%d]\n", width, height);
		Print_Image_Values(cb_image, width, height);
		printf("Cr LR Image1[%dx%d]\n", width, height);
		Print_Image_Values(cr_image, width, height);
		printf("\n");
	}
}

void RGB_to_Gray(unsigned char *rgb_image, unsigned char *g_image)
{
	int j;
	int index1, index2, index3;

	for (j = 0; j < LRdimension; j++)
	{
		index1 = 3 * j;
		index2 = 3 * j + 1;
		index3 = 3 * j + 2;
		g_image[j] = 16 + (YRC * rgb_image[index1]) + (YGC * rgb_image[index2])
				+ (YBC * rgb_image[index3]) + 0.5;
	}

}

void Normalize_Grayscale_Image(unsigned char *gray_image, int imagenumber, int dimension)
{
	int i;
	for (i = 0; i < dimension; i++)
	{
		Gray_Norm_LR_Image_Set[imagenumber][i] = (double) gray_image[i] / 255;
	}
}

void complex_multiplication(fftw_complex *result, fftw_complex *A, fftw_complex *B, int dim){ 
    int i;
        for (i=0;i<dim;i++) {
                        result[i][0] = (A[i][0]*B[i][0])+(-A[i][1]*B[i][1]);
                        result[i][1] = (A[i][0]*B[i][1])+(A[i][1]*B[i][0]);
    }
}

void normalize_matrix(fftw_complex *A, int dim){ //matrix element by element  divided by a constant
    int i;
        for (i=0;i<dim;i++) {
                        A[i][0] = A[i][0] / (dim);
    }
}

void calculate_R(fftw_complex *Result,fftw_complex *Ty, fftw_complex *Tx, fftw_complex *X, fftw_complex *Y, int width, int height) 
{
	int x, y;
	int coord;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			coord = (width*y)+x; //calculate vector coordinate
			Result[coord][0] = Ty[coord][0]*X[coord][0] - Tx[coord][0]*Y[coord][0];
			Result[coord][1] = 0;
		}
	}
}

double calculate_matrix_coefficient(fftw_complex *A, fftw_complex *B, int dim)
{	
	int i;
	double temporal;
	double sum = 0;
        for (i=0;i<dim;i++) {
 	       temporal = A[i][0] * B[i][0] ;
		sum = sum + temporal;
	}

        return sum;
}

void mtxebesub(fftw_complex *Result, fftw_complex *A, fftw_complex *B, int dim)
{ //Matrix element by element substraction.
    int i;
        for (i=0;i<dim;i++) {
                        Result[i][0] = A[i][0]-B[i][0];
    }
}

void Print_Image_Values(unsigned char *vector, int w, int h)
{
	int ii, jj;
	for (ii = 0; ii < h; ii++)
	{
		for (jj = 0; jj < w; jj++)
		{
			printf("M[%3d][%3d] = %3d ", ii, jj, vector[(w * ii) + jj]);
		}
		printf("\n");
	}
	printf("\n");
}

void Print_dMatrix(double **matrix, int w, int h)
{
	int ii, jj;
	for (ii = 0; ii < h; ii++)
	{
		for (jj = 0; jj < w; jj++)
		{
			printf("M[%d][%d] = %2.5f ", ii, jj, matrix[ii][jj]);
		}
		printf("\n");
	}
	printf("\n");
}

void Print_dVector(double *vector, int w, int h)
{
	int ii, jj;
	for (ii = 0; ii < h; ii++)
{
		for (jj = 0; jj < w; jj++)
		{
			printf("M[%d][%d] = %2.6f ", ii, jj, vector[(w*ii)+jj]);
		}
		printf("\n");
	}
	printf("\n");
}

void mtxinv(double *Ai, double *A)
{ 
    double adj[9];
    double det;
    adj[0] = A[4]*A[8]-A[5]*A[7]; adj[1] = A[2]*A[7]-A[1]*A[8]; adj[2] = A[1]*A[5]-A[2]*A[4];
    adj[3] = A[5]*A[6]-A[3]*A[8]; adj[4] = A[0]*A[8]-A[2]*A[6]; adj[5] = A[2]*A[3]-A[0]*A[5];
    adj[6] = A[3]*A[7]-A[4]*A[6]; adj[7] = A[1]*A[6]-A[0]*A[7]; adj[8] = A[0]*A[4]-A[1]*A[3];
    det=A[0]*adj[0]-A[1]*(-adj[3])+A[2]*adj[6];
    Ai[0]=adj[0]/det; Ai[1]=adj[1]/det; Ai[2]=adj[2]/det;
    Ai[3]=adj[3]/det; Ai[4]=adj[4]/det; Ai[5]=adj[5]/det;
    Ai[6]=adj[6]/det; Ai[7]=adj[7]/det; Ai[8]=adj[8]/det;
}

void  mtxmult(double *s, double *A, double *B)
{ //element by element matrix multiplication
        s[0]=A[0]*B[0] + A[1]*B[1] + A[2]*B[2];
        s[1]=A[3]*B[0] + A[4]*B[1] + A[5]*B[2];
        s[2]=A[6]*B[0] + A[7]*B[1] + A[8]*B[2];
}

void Print_fftwVector(fftw_complex *vector, int w, int h)
{
	int ii,jj;
	for (ii=0;ii<h;ii++)
	{
		for (jj=0;jj<w;jj++)
		{
			printf("M[%3d][%3d] = { %2.6f, %2.6f } ",ii,jj,vector[(w*ii)+jj][0],vector[(w*ii)+jj][1]);
		}
		printf("\n");
	}
	printf("\n");
}


void print_pointVector(point *vector, int dim) {
        int i;
          for (i=0;i<dim;i++) {
                     printf("M[%3d] = {%f,%f,%f}\n",i,vector[i].y,vector[i].x,vector[i].z);

          }
}


void print_pointMatrix(point *matrix, int sizeh, int sizew) {
        int i,j;
          for (i=0;i<sizeh;i++) {
                  for   (j=0;j<sizew;j++){
                     printf("M[%3d,%3d] = {%f,%f,%f} ",i,j,matrix[i*sizew+j].y,matrix[i*sizew+j].x,matrix[i*sizew+j].z);
                  }
		  printf("\n");
          }
}

int Construct_HR_Bitmap_Image(unsigned char *Y_Channel, unsigned char *Cb_Channel, unsigned char *Cr_Channel, int dimension) {


        char TemporalString[MAX_PATH_SIZE] = "";        
        //Initialize 3-Channel Grayscale Image
        HR_3_Channels_Image = Init_ucVector(3 * dimension);        


        if (Grayscale) 
        {
                //Construct Luminance Bitmap Image
                Construct_Grayscale_Image(Y_Channel, HR_3_Channels_Image);
                strcpy(TemporalString, HRImagesPath); 
	        strcat(TemporalString, "Y.bmp");
                write_bmp_file(TemporalString, HR_3_Channels_Image);
                
                //Construct blue chrominance Bitmap Image
                Construct_Grayscale_Image(Cb_Channel, HR_3_Channels_Image);
                strcpy(TemporalString, HRImagesPath); 
	        strcat(TemporalString, "Cb.bmp");
                write_bmp_file(TemporalString, HR_3_Channels_Image);

                //Construct red chrominance Bitmap Image
                Construct_Grayscale_Image(Cr_Channel, HR_3_Channels_Image);
                strcpy(TemporalString, HRImagesPath); 
	        strcat(TemporalString, "Cr.bmp");
                write_bmp_file(TemporalString, HR_3_Channels_Image);
        }

        //Construct HR Color Bitmap Image
        YCbCr_to_RGB(HR_3_Channels_Image, Y_Channel, Cb_Channel, Cr_Channel);
        strcpy(TemporalString, HRImagesPath); 
	strcat(TemporalString, "aColorImage.bmp");
        write_bmp_file(TemporalString, HR_3_Channels_Image);
  
        printf("Done\n");
        free(HR_3_Channels_Image);

	return (_TRUE_);

}

void Construct_Grayscale_Image(unsigned char *Y_component, unsigned char *Grayscale_Bitmap_Image) {
        int i;
        for (i=0; i<HRdimension;i++) {

          Grayscale_Bitmap_Image[3*i] = Y_component[i];
          Grayscale_Bitmap_Image[3*i+1] = Y_component[i];
          Grayscale_Bitmap_Image[3*i+2] = Y_component[i];
 
       }

}

int write_bmp_file( char *filename, unsigned char *Grayscale_Bitmap_Image) {

        BMPHEAD bh;

        memset ((char *)&bh,0,sizeof(BMPHEAD)); /* sets everything to 0 */

        if (verbose == 21) 
                printf("sizeof(BMPHEAD) = %zu\n", sizeof(BMPHEAD));

        //bh.filesize  =   calculated size of your file (see below)
        //bh.reserved  = two zero bytes
        bh.headersize  = 54L;//  (for 24 bit images)
        bh.infoSize  =  0x28L;//  (for 24 bit images)
        bh.width     = HRwidth ;//in pixels of your image
        bh.depth     = HRheight;// in pixels of your image
        bh.biPlanes  =  1 ;//(for 24 bit images)
        bh.bits      = 24 ;//(for 24 bit images)
        bh.biCompression = 0L;;//  (no compression)

        if (verbose == 21) {
                printf("bh.headersize = %d\n", bh.headersize);
                printf("bh.infoSize = %d\n", bh.infoSize);
                printf("bh.width = %d\n", bh.width);
                printf("bh.depth = %d\n", bh.depth);
                printf("bh.biplanes = %hd\n", bh.biPlanes);
                printf("bh.bits = %hd\n", bh.bits);
                printf("bh.biCompression = %d\n", bh.biCompression);                
        }
        int bytesPerLine;

        bytesPerLine = HRwidth * 3;  /* (for 24 bit images) */

        if (verbose == 21) 
                printf("bytesPerLine = %x\n", bytesPerLine);
 
        /* round up to a dword boundary */
        if (bytesPerLine & 0x0003)
        {
                bytesPerLine |= 0x0003;
        ++bytesPerLine;
        }

        if (verbose == 21) 
                printf("bytesPerLine = %x\n", bytesPerLine);

        bh.filesize = bh.headersize+(long)bytesPerLine*bh.depth;

        if (verbose == 21) 
                printf("bh.filesize = %d\n", bh.filesize);

        FILE * bmpfile;

        //printf("Bytes per line : %d\n", bytesPerLine);

        bmpfile = fopen(filename, "wb");
 
        if (bmpfile == NULL)
        {
                printf("Error opening output file\n");
                /* -- close all open files and free any allocated memory -- */
                exit (1);
        }
 
        fwrite("BM",1,2,bmpfile);
        fwrite((char *)&bh, 1, sizeof (bh), bmpfile);

        char *linebuf;

        linebuf = (char *) calloc(1, bytesPerLine);
 
        if (linebuf == NULL)
        {
                printf ("Error allocating memory\n");
                free(Grayscale_Bitmap_Image);
                /* -- close all open files and free any allocated memory -- */
                exit (1);
        }


        int line,x;

        for (line = HRheight-1; line >= 0; line --)
        {
                /* fill line linebuf with the image data for that line */
                for( x =0 ; x < HRwidth; x++ )
                {
                        *(linebuf+x*bytes_per_pixel) = *(Grayscale_Bitmap_Image+(x+line*HRwidth)*bytes_per_pixel+2);
                        *(linebuf+x*bytes_per_pixel+1) = *(Grayscale_Bitmap_Image+(x+line*HRwidth)*bytes_per_pixel+1);
                        *(linebuf+x*bytes_per_pixel+2) = *(Grayscale_Bitmap_Image+(x+line*HRwidth)*bytes_per_pixel+0);
                }

        /* remember that the order is BGR and if HRwidth is not a multiple
        of 4 then the last few bytes may be unused
        */

        fwrite(linebuf, 1, bytesPerLine, bmpfile);
        
        }

        free(linebuf);
        fclose(bmpfile);

        return (_TRUE_);
}

unsigned char bilinear_interpolate(unsigned char *the_image, double x, double y) {

        double  fraction_x, fraction_y, one_minus_x, one_minus_y, tmp_double;
        double  p1,p2;
        int     ceil_x, ceil_y, floor_x, floor_y;
        unsigned char   p3; 
        unsigned char   result = CHFILL;

        if (EdgeBorderType == _DUPLICATE_EDGE_) {
                if ( x < 0.0) 
                        x = 0.0;
                else if (x > (double)(width-1))
                        x = (double) (width-1);
                
                if (y < 0.0)
                        y = 0.0;
                else if ( y > (double)(height-1) )
                        y = (double) (height-1);
        }
        else {
                if( x < 0.0 || x > (double)(width-1) || y < 0.0 || y > (double)(height-1) )
                        return(result);
        }


        tmp_double = floor(x);
        floor_x    = tmp_double;
 
        tmp_double = floor(y);
        floor_y    = tmp_double;
   
        tmp_double = ceil(x);
        ceil_x     = tmp_double;

        tmp_double = ceil(y);
        ceil_y     = tmp_double;

        fraction_x = x - floor(x);
        fraction_y = y - floor(y);

        one_minus_x = 1.0 - fraction_x;
        one_minus_y = 1.0 - fraction_y;

        tmp_double = one_minus_x * (double)(the_image[floor_y*width + floor_x]) + fraction_x * (double)(the_image[floor_y*width + ceil_x]);
        p1         = tmp_double;
        tmp_double = one_minus_x * (double)(the_image[ceil_y*width + floor_x]) + fraction_x * (double)(the_image[ceil_y*width + ceil_x]);
        p2         = tmp_double;
        tmp_double = one_minus_y * (double)(p1) + fraction_y * (double)(p2);
        p3         = (tmp_double);
        return(p3);

}  /* ends bilinear_interpolate */

int Image_Resize(unsigned char *Image, unsigned char *Resized_Image) {

        int     i,j;
        int     modx, mody, intx, inty; 
        int     LRcoord, HRcoord;
        double  tmpx, tmpy;
        float   fi, fj;

        for(i = 0; i < HRheight; i++) {
                for(j=0; j < HRwidth; j++) {

                        modx = (j%factor);
                        mody = (i%factor);

                        if ((modx == 0) && (mody == 0)) {
                                intx = (int) (j/factor);
                                inty = (int) (i/factor);
                                HRcoord = i*HRwidth + j;
                                LRcoord = inty*width + intx;
                                Resized_Image[HRcoord] =  Image[LRcoord];
                        }
                        else 
                        {
                                fi = i;
                                fj = j;
                                tmpx = (fj/factor);
                                tmpy = (fi/factor);
                                HRcoord = i*HRwidth + j;
                                Resized_Image[HRcoord] =  bilinear_interpolate(Image, tmpx, tmpy);
                        }
 
                }
        }

	return (_TRUE_);

}

int Denormalize_Color_Channel(point *Normalized_Image_Channel, unsigned char *Denormalized_Image_Channel, int dimension) {

        int i;
        double temp;
        int bd; //border width
                                      
        //Denormalize Luminance Channel        
	for (i=0;i<dimension;i++) {
                //If pixel value is not a number
          	if (isnan(Normalized_Image_Channel[i].z)) {
                        temp = 255;
                        Denormalized_Image_Channel[i] = (unsigned char) (temp);
                }
                //if pixel is a number then denormalize                 
                else {
                        temp = (Normalized_Image_Channel[i].z * 255) + 0.5;
                        if (temp < 0)   temp = 0;
                        if (temp > 255)  temp = 255;
                        Denormalized_Image_Channel[i] = (unsigned char) (temp);
                     }
         }

        switch(EdgeBorderType){
                case _DUPLICATE_EDGE_: 
                        bd = factor-1; //border width
                        Duplicate_Border_Pixels(Denormalized_Image_Channel, bd, HRwidth, HRheight, _DUPLICATE_EDGE_);
                        break;
                case _FILLED_EDGE_: 
                        bd = factor-1; //border width
                        Duplicate_Border_Pixels(Denormalized_Image_Channel, bd, HRwidth, HRheight, _FILLED_EDGE_);
                        break;
                default:
                        EdgeBorderType = _NO_EDGE_; //Set default border to none
                        break;
        }

        if (verbose == 20)
        {
		printf("Denormalized_Image_Channel1[%dx%d]\n", HRwidth, HRheight);
		Print_Image_Values(Denormalized_Image_Channel, HRwidth, HRheight);
        }

        
        return (_TRUE_);

}

int YCbCr_to_RGB(unsigned char *RGB_Image, unsigned char *Y_Channel, unsigned char *Cb_Channel,unsigned char *Cr_Channel) {

        int j;
        double doubletemp;

        for ( j=0; j < HRdimension; j++ ) {

                doubletemp = (RYC * (Y_Channel[j] - 16)) +                              (RCRC * (Cr_Channel[j] - 128));
                if (doubletemp < 0) 
                        doubletemp = 0;
                if (doubletemp > 255) 
                        doubletemp = 255;
                RGB_Image[3*j] = doubletemp;

                doubletemp = (GYC * (Y_Channel[j] - 16)) - (GCBC * (Cb_Channel[j]-128)) - (GCRC * (Cr_Channel[j] - 128));
                if (doubletemp < 0) 
                        doubletemp = 0;
                if (doubletemp > 255) 
                        doubletemp = 255;
                RGB_Image[3*j+1] = doubletemp;

                doubletemp = (BYC * (Y_Channel[j] - 16)) + (BCBC * (Cb_Channel[j]-128));
                if (doubletemp < 0) 
                        doubletemp = 0;
                if (doubletemp > 255) 
                        doubletemp = 255;
                RGB_Image[3*j+2] = doubletemp;

        }

     return (_TRUE_);

}

int Duplicate_Border_Pixels(unsigned char *the_image, int border_width, int image_width, int image_height, int BorderType) {
        
        int i,j;
        int coord; //coordinate
        int matrix_width, matrix_height;

        if (BorderType == _DUPLICATE_EDGE_) {  

                int coord_dup; //coordinate of pixel to duplicate

                for (i = 0; i < border_width; i++) {
                        for (j = 0; j < (image_width-1); j++) {
                                coord = (image_width*i)+j;
                                        coord_dup = (image_width*(i + border_width)) + j; 
                                        the_image[coord] = the_image[coord_dup];
                        }
                }

                matrix_height = image_height - 1;
                for (i = matrix_height; i > matrix_height - border_width; i--) {
                        for (j = 0; j < (image_width-1); j++) {
                                coord = (image_width*i)+j;
                                       coord_dup = (image_width*(i - border_width)) + j; 
                                       the_image[coord] = the_image[coord_dup];
                        }
                }

                for (i = 0; i < image_height; i++) {
                        for (j = 0; j < border_width; j++) {
                                coord = (image_width*i)+j;
                                coord_dup = (image_width*i) + j + border_width; 
                                the_image[coord] = the_image[coord_dup];  
                      }
                }

                matrix_width = image_width - 1;
                for (i = 0; i < image_height; i++) {
                        for (j = matrix_width; j > matrix_width - border_width; j--) {
                                coord = (image_width*i)+j;
                                coord_dup = (image_width*i) + j - border_width; 
                                the_image[coord] = the_image[coord_dup];
                        }
                }
        }
        else if (BorderType == _FILLED_EDGE_) {

                for (i = 0; i < border_width; i++) {
                        for (j = 0; j < (image_width-1); j++) {
                                coord = (image_width*i)+j;
                                the_image[coord] = YFILL;
                        }
                }

                matrix_height = image_height - 1;
                for (i = matrix_height; i > matrix_height - border_width; i--) {
                        for (j = 0; j < (image_width-1); j++) {
                                coord = (image_width*i)+j;
                                the_image[coord] = YFILL;
                        }
                }

                for (i = 0; i < image_height; i++) {
                        for (j = 0; j < border_width; j++) {
                                coord = (image_width*i)+j;
                                the_image[coord] = YFILL;
                      }
                }

                matrix_width = image_width - 1;
                for (i = 0; i < image_height; i++) {
                        for (j = matrix_width; j > matrix_width - border_width; j--) {
                                coord = (image_width*i)+j;
                                the_image[coord] = YFILL;
                        }
                }

        }  
        
        return (_TRUE_);        

}

static void usage(unsigned char long_usage)
{

    printf("Usage: nnsr -p <ImagesPath>\n");
    printf("            -o <HRImagesPath>\n");
    printf("            [-v <verbose>] \n");
    printf("            [-f <factor>] \n");
    printf("            [-n <NumLRimages>]\n");
    printf("            [-m <TestMEVectors>]\n");
    printf("            [-z <ZeroTolerance>]\n");
    printf("            [-b <EdgeBorderType>]\n");
    printf("            [-t <Thingrid >]\n");
    printf("            [-G]\n");
    printf("            [-E]\n");
    if(!long_usage)
            printf("Use paramter -H for more information\n"); 
    else    
    {
            printf("Options:\n");
            printf("  -p <ImagesPath>     -- low resolution input images path\n");
            printf("  -o <ImagesPath>     -- high resolution output images path\n");
            printf("  -v <verbose>        -- verbose level for debugging purposes (0:no verbose 1-19: verbose debug) \n");
            printf("  -f <factor>         -- super-resolution up-scale factor\n");
            printf("  -n <NumLRimages>    -- number of low-resolution images used for interpolation\n");
            printf("  -m <TestMEVectors>  -- Set predefined motion estimation vectors for \n");
            printf("                         interpolation. This parameter is for debugging purposes\n");
            printf("  -z <ZeroTolerance>  -- Estimated angle tolerance band in order to round to zero. This is required\n");
            printf("                         when the estimated angle is close to zero and interpolation\n");
            printf("                         algorithm returns error\n");
            printf("  -b <EdgeBorderType> -- set the border type. 0: No border,\n");  
            printf("                                              1: Duplicate pixels\n");
            printf("                                              2: Fill edge border\n");
            printf("  -t <Thingrid>       -- thin input images by averaging pixel values within\n"); 
            printf("                         clusters of consecutive input points such that the\n");
            printf("                         sum of distances between points within each cluster\n");
            printf("                         does not exceed the specified maximum value\n");
            printf("  -G                  -- Generate luminance, blue chrominance and red chrominance\n"); 
            printf("                         channel images\n");
            printf("  -E                  -- Performs extrapolation in the edge pixels\n"); 
    }    
}
