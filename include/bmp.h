/**
 * bmp.h
 */

#if !defined (_BMP_H)
# define _BMP_H

# include <stdint.h>
# include <stdbool.h>
# include <assert.h>

# if !defined(_cleanup)
#  define	_cleanup(_x)	__attribute__((cleanup(_x)))
# endif

# if !defined(_packed)
#  define	_packed			__attribute__((packed))
# endif

typedef struct	_bmpfile		BMPfile;

typedef struct	_bmpfheader		BMPfile_header;
typedef struct	_bmpiheader		BMPinfo_header;

typedef enum	_bmpcolorformat	BMPcolor_format;
typedef enum 	_bmpcompression	BMPcompression;

typedef struct	_bmppalentry	BMPpal_entry;
typedef struct	_bmppal			BMPpal;

typedef union	_bmpcolor	BMPcolor;

struct _bmpfile
{
	int32_t		fd;
	uint32_t	size;
	uint8_t		*content;
};

struct _bmpfheader
{
	uint16_t	signature;
	uint32_t	file_size;
	uint32_t	reserved;
	uint32_t	data_offset;
}	_packed;

enum _bmpcolorformat
{
	BMP_MONOCHROME		= 1,
	BMP_4_BIT_PALETTE	= 4,
	BMP_8_BIT_PALETTE	= 8,
	BMP_16_BIT_RGB		= 16,
	BMP_24_BIT_RGB		= 24,
};

enum _bmpcompression
{
	BMP_COMPRESSION_NONE,
	BMP_COMPRESSION_8_BIT_RLE,
	BMP_COMPRESSION_4_BIT_RLE,
};

struct _bmpiheader
{
	uint32_t	header_size;
	uint32_t	width;
	uint32_t	height;
	uint16_t	planes;
	uint16_t	bpp;
	uint32_t	compression;
	uint32_t	image_size;
	uint32_t	x_ppm;
	uint32_t	y_ppm;
	uint32_t	used_colors;
	uint32_t	important_colors;
}	_packed;

/* strides */

struct _bmppalentry
{
	uint8_t	r;
	uint8_t	g;
	uint8_t	b;
	uint8_t	reserved;
}	_packed;

struct _bmppal
{
	uint32_t		size;
	BMPpal_entry	*grid;
};

typedef struct _bmp_bgr24	BMPbgr24;
typedef struct _bmp_bgr16	BMPbgr16;
typedef struct _bmp_pal8	BMPpal8;
typedef struct _bmp_pal4	BMPpal4;
typedef struct _bmp_pal1	BMPpal1;

union _bmpcolor
{
	uint32_t	argb;
	struct
	{
		uint8_t	b;
		uint8_t	g;
		uint8_t	r;
		uint8_t	a;
	};
};

struct _bmp_bgr24
{
	uint8_t	b;
	uint8_t	g;
	uint8_t	r;
}	_packed;

struct _bmp_bgr16
{
	uint8_t	reserved:1;
	uint8_t	b:5;
	uint8_t	g:5;
	uint8_t	r:5;
}	_packed;

struct _bmp_pal8
{
	uint8_t	idx;
}	_packed;

struct _bmp_pal4
{
	uint8_t	idx0:4;
	uint8_t	idx1:4;
}	_packed;

struct _bmp_pal1
{
	uint8_t	idx0:1;
	uint8_t	idx1:1;
	uint8_t	idx2:1;
	uint8_t	idx3:1;
	uint8_t	idx4:1;
	uint8_t	idx5:1;
	uint8_t	idx6:1;
	uint8_t	idx7:1;
}	_packed;

bool
bmp_open(const char *filepath, BMPfile *image);

void
bmp_close(BMPfile *file);

uint32_t
*bmp_parse(const BMPfile *image, uint32_t *width, uint32_t *height, uint32_t *channels);

#endif // _BMP_H

#define BMP_IMPLEMENTATION
#if defined(BMP_IMPLEMENTATION)

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <fcntl.h>
# include <unistd.h>
# include <sys/mman.h>
# include <sys/stat.h>

static inline void
bmp_close_fd(int32_t *fd)
{
	if (fd && *fd > 2)
		close(*fd);
}

# define	BMP_MIN_SIZE		(long)(sizeof(BMPfile_header) + sizeof(BMPinfo_header))
# define	BMP_INFOHDR_OFFSET	sizeof(BMPfile_header)

bool
bmp_open(const char *filepath, BMPfile *image)
{
	if (!image || !filepath)		return false;
	if (image->content)				return false;
	if (image->fd != 0)				return false;
	if (image->size != 0)			return false;

	_cleanup(bmp_close_fd)	int32_t	fd = open(filepath, O_RDONLY);
	struct stat						st = {0};
	void							*content; 

	if (fd == -1)					return false;
	if (fstat(fd, &st) == -1)		return false;
	if (st.st_size <= BMP_MIN_SIZE)	return false;

	content = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	if (content == MAP_FAILED)		return false;

	*image = (BMPfile)
	{
		.fd			= fd,
		.size		= st.st_size,
		.content	= content,
	};
	return true;
}

void
bmp_close(BMPfile *image)
{
	if (image)
	{
		munmap(image->content, image->size);
		image->content = NULL;
		image->size = 0;
		bmp_close_fd(&image->fd);
	}
}

bool
bmp__file_header(const BMPfile *file, BMPfile_header *hdr)
{
	BMPfile_header	*fhdr;

	fhdr = (BMPfile_header *) file->content;
	
	if (fhdr->signature != 0x4d42)		return false;
	if (fhdr->file_size != file->size)	return false;
	if (fhdr->reserved != 0)			return false;

	*hdr = *fhdr;
	return true;
}

bool
bmp__info_header(const BMPfile *file, BMPinfo_header *hdr)
{
	BMPinfo_header	*ihdr;

	ihdr = (BMPinfo_header *)(file->content + BMP_INFOHDR_OFFSET);

	if (ihdr->header_size != sizeof(BMPinfo_header))	return false;
	if (ihdr->width == 0 || ihdr->height == 0)			return false;
	if (ihdr->planes != 1)								return false;
	switch (ihdr->bpp)
	{
		case 1: case 4: case 8: case 16: case 24:
			break ;
		default:
			return false;
	}
	if (ihdr->compression > 2)							return false;
	if (ihdr->compression && ihdr->image_size == 0)		return false;	

	*hdr = *ihdr;
	return true;
}

bool
bmp__color_palette(const BMPfile *file, BMPinfo_header *ihdr, BMPpal *palette)
{
	switch (ihdr->bpp)
	{
		case BMP_MONOCHROME:
		case BMP_4_BIT_PALETTE:
		case BMP_8_BIT_PALETTE:
		case BMP_16_BIT_RGB:
			break ;
		case BMP_24_BIT_RGB:
			if (ihdr->height * 3 * ihdr->width != ihdr->image_size)	return false;
			break ;
		default:
			assert(0 && "this image should't have a palette. aborting...");
	}

	palette->size = ihdr->used_colors;
	palette->grid = malloc(sizeof(BMPpal_entry) * palette->size);

	if (!palette->grid)	return false;

	BMPpal_entry	*raw = (BMPpal_entry *)(file->content + BMP_MIN_SIZE);

	memcpy(palette->grid, raw, palette->size * sizeof(BMPpal_entry));

	for (uint32_t i = 0; i < palette->size; ++i)
	{
		BMPpal_entry	*entry = &palette->grid[i];

//		printf("(0x%02x%02x%02x)\n", entry->r, entry->g, entry->b);
	}

	return true;
}

uint32_t
*bmp__extract_pixels_bgr24(uint8_t *raw, const BMPinfo_header *ihdr)
{
	uint32_t	u8_width;
	uint32_t	u8_height;

	u8_width = ((3 * ihdr->width) + 3) & ~3;
	u8_height = ihdr->height;

	BMPcolor	*pixels = malloc(sizeof(BMPcolor) * ihdr->width * ihdr->height);
	
	for (int i = u8_height - 1; i >= 0; --i)
	{
		BMPbgr24	*data = (BMPbgr24 *)(raw + ((u8_height - i - 1) * u8_width));

		for (unsigned int j = 0; j < ihdr->width; ++j)
		{
			pixels[i * ihdr->height + j] = (BMPcolor)
			{
				.a = 0xff,
				.r = data->r,
				.b = data->b,
				.g = data->g,
			};
			data++;
 		}
	}
	return ((uint32_t *)pixels);
}

uint32_t
*bmp__extract_pixels(uint8_t *raw, const BMPinfo_header *ihdr)
{
	switch (ihdr->bpp)
	{
		case BMP_MONOCHROME:
		case BMP_4_BIT_PALETTE:
		case BMP_8_BIT_PALETTE:
		case BMP_16_BIT_RGB:
		default:
			assert(0 && "TODO : manage other bit-per-pixel values.");
		case BMP_24_BIT_RGB:
			return bmp__extract_pixels_bgr24(raw, ihdr);
	}
	return (NULL);
}

uint32_t
*bmp_parse(const BMPfile *image, uint32_t *width, uint32_t *height, uint32_t *channels)
{
	BMPfile_header	fhdr = {0};
	BMPinfo_header	ihdr = {0};
	BMPpal			palette = {0};

	if (!bmp__file_header(image, &fhdr))	return NULL;
	if (!bmp__info_header(image, &ihdr))	return NULL;

	if (!bmp__color_palette(image, &ihdr, &palette))	return NULL;

	uint32_t	*image_data;

	image_data = bmp__extract_pixels(image->content + fhdr.data_offset, &ihdr);

	*width = ihdr.width;
	*height = ihdr.height;
	*channels = 4;

	return (image_data);
}

#endif // BMP_IMPLEMENTATION
