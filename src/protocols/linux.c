#include "protocols/linux.h"
#include "shellerr.h"
#include "screen.h"
#include "lib/edid.h"
#include "lib/acpi.h"

// The implementation of this Linux loader code was taken from the Limine
// bootloader with modifications and adaptations for this boot manager
// https://github.com/limine-bootloader/limine

#define DIV_ROUNDUP(a, b) (((a) + ((b) - 1)) / (b))

inline uint64_t AlignUp(uint64_t value, uint64_t align)
{
	return DIV_ROUNDUP(value, align) * align;
}

inline uint64_t AlignDown(uint64_t value, uint64_t align)
{
	return (value / align) * align;
}

// The following definitions were copied from the Linux kernel
// licensed under GPL-2.0 WITH Linux-syscall-note 

/* linux/screen_info.h */
struct screen_info {
	uint8_t  orig_x;		/* 0x00 */
	uint8_t  orig_y;		/* 0x01 */
	uint16_t ext_mem_k;	/* 0x02 */
	uint16_t orig_video_page;	/* 0x04 */
	uint8_t  orig_video_mode;	/* 0x06 */
	uint8_t  orig_video_cols;	/* 0x07 */
	uint8_t  flags;		/* 0x08 */
	uint8_t  unused2;		/* 0x09 */
	uint16_t orig_video_ega_bx;/* 0x0a */
	uint16_t unused3;		/* 0x0c */
	uint8_t  orig_video_lines;	/* 0x0e */
	uint8_t  orig_video_isVGA;	/* 0x0f */
	uint16_t orig_video_points;/* 0x10 */

	/* VESA graphic mode -- linear frame buffer */
	uint16_t lfb_width;	/* 0x12 */
	uint16_t lfb_height;	/* 0x14 */
	uint16_t lfb_depth;	/* 0x16 */
	uint32_t lfb_base;		/* 0x18 */
	uint32_t lfb_size;		/* 0x1c */
	uint16_t cl_magic, cl_offset; /* 0x20 */
	uint16_t lfb_linelength;	/* 0x24 */
	uint8_t  red_size;		/* 0x26 */
	uint8_t  red_pos;		/* 0x27 */
	uint8_t  green_size;	/* 0x28 */
	uint8_t  green_pos;	/* 0x29 */
	uint8_t  blue_size;	/* 0x2a */
	uint8_t  blue_pos;		/* 0x2b */
	uint8_t  rsvd_size;	/* 0x2c */
	uint8_t  rsvd_pos;		/* 0x2d */
	uint16_t vesapm_seg;	/* 0x2e */
	uint16_t vesapm_off;	/* 0x30 */
	uint16_t pages;		/* 0x32 */
	uint16_t vesa_attributes;	/* 0x34 */
	uint32_t capabilities;     /* 0x36 */
	uint32_t ext_lfb_base;	/* 0x3a */
	uint8_t  _reserved[2];	/* 0x3e */
} __attribute__((packed));

#define VIDEO_TYPE_MDA		0x10	/* Monochrome Text Display	*/
#define VIDEO_TYPE_CGA		0x11	/* CGA Display 			*/
#define VIDEO_TYPE_EGAM		0x20	/* EGA/VGA in Monochrome Mode	*/
#define VIDEO_TYPE_EGAC		0x21	/* EGA in Color Mode		*/
#define VIDEO_TYPE_VGAC		0x22	/* VGA+ in Color Mode		*/
#define VIDEO_TYPE_VLFB		0x23	/* VESA VGA in graphic mode	*/

#define VIDEO_TYPE_PICA_S3	0x30	/* ACER PICA-61 local S3 video	*/
#define VIDEO_TYPE_MIPS_G364	0x31    /* MIPS Magnum 4000 G364 video  */
#define VIDEO_TYPE_SGI          0x33    /* Various SGI graphics hardware */

#define VIDEO_TYPE_TGAC		0x40	/* DEC TGA */

#define VIDEO_TYPE_SUN          0x50    /* Sun frame buffer. */
#define VIDEO_TYPE_SUNPCI       0x51    /* Sun PCI based frame buffer. */

#define VIDEO_TYPE_PMAC		0x60	/* PowerMacintosh frame buffer. */

#define VIDEO_TYPE_EFI		0x70	/* EFI graphic mode		*/

#define VIDEO_FLAGS_NOCURSOR	(1 << 0) /* The video mode has no cursor set */

#define VIDEO_CAPABILITY_SKIP_QUIRKS	(1 << 0)
#define VIDEO_CAPABILITY_64BIT_BASE	(1 << 1)	/* Frame buffer base is 64-bit */
/* linux/screen_info.h */

/* linux/apm_bios.h */
struct apm_bios_info {
	uint16_t	version;
	uint16_t	cseg;
	uint32_t	offset;
	uint16_t	cseg_16;
	uint16_t	dseg;
	uint16_t	flags;
	uint16_t	cseg_len;
	uint16_t	cseg_16_len;
	uint16_t	dseg_len;
};
/* linux/apm_bios.h */

/* linux/edd.h */
#define EDDMAXNR 6		/* number of edd_info structs starting at EDDBUF  */
#define EDD_MBR_SIG_MAX 16        /* max number of signatures to store */

struct edd_device_params {
	uint16_t length;
	uint16_t info_flags;
	uint32_t num_default_cylinders;
	uint32_t num_default_heads;
	uint32_t sectors_per_track;
	uint64_t number_of_sectors;
	uint16_t bytes_per_sector;
	uint32_t dpte_ptr;		/* 0xFFFFFFFF for our purposes */
	uint16_t key;		/* = 0xBEDD */
	uint8_t device_path_info_length;	/* = 44 */
	uint8_t reserved2;
	uint16_t reserved3;
	uint8_t host_bus_type[4];
	uint8_t interface_type[8];
	union {
		struct {
			uint16_t base_address;
			uint16_t reserved1;
			uint32_t reserved2;
		} __attribute__ ((packed)) isa;
		struct {
			uint8_t bus;
			uint8_t slot;
			uint8_t function;
			uint8_t channel;
			uint32_t reserved;
		} __attribute__ ((packed)) pci;
		/* pcix is same as pci */
		struct {
			uint64_t reserved;
		} __attribute__ ((packed)) ibnd;
		struct {
			uint64_t reserved;
		} __attribute__ ((packed)) xprs;
		struct {
			uint64_t reserved;
		} __attribute__ ((packed)) htpt;
		struct {
			uint64_t reserved;
		} __attribute__ ((packed)) unknown;
	} interface_path;
	union {
		struct {
			uint8_t device;
			uint8_t reserved1;
			uint16_t reserved2;
			uint32_t reserved3;
			uint64_t reserved4;
		} __attribute__ ((packed)) ata;
		struct {
			uint8_t device;
			uint8_t lun;
			uint8_t reserved1;
			uint8_t reserved2;
			uint32_t reserved3;
			uint64_t reserved4;
		} __attribute__ ((packed)) atapi;
		struct {
			uint16_t id;
			uint64_t lun;
			uint16_t reserved1;
			uint32_t reserved2;
		} __attribute__ ((packed)) scsi;
		struct {
			uint64_t serial_number;
			uint64_t reserved;
		} __attribute__ ((packed)) usb;
		struct {
			uint64_t eui;
			uint64_t reserved;
		} __attribute__ ((packed)) i1394;
		struct {
			uint64_t wwid;
			uint64_t lun;
		} __attribute__ ((packed)) fibre;
		struct {
			uint64_t identity_tag;
			uint64_t reserved;
		} __attribute__ ((packed)) i2o;
		struct {
			uint32_t array_number;
			uint32_t reserved1;
			uint64_t reserved2;
		} __attribute__ ((packed)) raid;
		struct {
			uint8_t device;
			uint8_t reserved1;
			uint16_t reserved2;
			uint32_t reserved3;
			uint64_t reserved4;
		} __attribute__ ((packed)) sata;
		struct {
			uint64_t reserved1;
			uint64_t reserved2;
		} __attribute__ ((packed)) unknown;
	} device_path;
	uint8_t reserved4;
	uint8_t checksum;
} __attribute__ ((packed));

struct edd_info {
	uint8_t device;
	uint8_t version;
	uint16_t interface_support;
	uint16_t legacy_max_cylinder;
	uint8_t legacy_max_head;
	uint8_t legacy_sectors_per_track;
	struct edd_device_params params;
} __attribute__ ((packed));
/* linux/edd.h */

/* asm/ist.h */
struct ist_info {
	uint32_t signature;
	uint32_t command;
	uint32_t event;
	uint32_t perf_level;
};
/* asm/ist.h */

/* video/edid.h */
struct edid_info {
	unsigned char dummy[128];
};
/* video/edid.h */

/* asm/bootparam.h */
struct setup_header {
	uint8_t	setup_sects;
	uint16_t	root_flags;
	uint32_t	syssize;
	uint16_t	ram_size;
	uint16_t	vid_mode;
	uint16_t	root_dev;
	uint16_t	boot_flag;
	uint16_t	jump;
	uint32_t	header;
	uint16_t	version;
	uint32_t	realmode_swtch;
	uint16_t	start_sys_seg;
	uint16_t	kernel_version;
	uint8_t	type_of_loader;
	uint8_t	loadflags;
	uint16_t	setup_move_size;
	uint32_t	code32_start;
	uint32_t	ramdisk_image;
	uint32_t	ramdisk_size;
	uint32_t	bootsect_kludge;
	uint16_t	heap_end_ptr;
	uint8_t	ext_loader_ver;
	uint8_t	ext_loader_type;
	uint32_t	cmd_line_ptr;
	uint32_t	initrd_addr_max;
	uint32_t	kernel_alignment;
	uint8_t	relocatable_kernel;
	uint8_t	min_alignment;
	uint16_t	xloadflags;
	uint32_t	cmdline_size;
	uint32_t	hardware_subarch;
	uint64_t	hardware_subarch_data;
	uint32_t	payload_offset;
	uint32_t	payload_length;
	uint64_t	setup_data;
	uint64_t	pref_address;
	uint32_t	init_size;
	uint32_t	handover_offset;
	uint32_t	kernel_info_offset;
} __attribute__((packed));

struct sys_desc_table {
	uint16_t length;
	uint8_t  table[14];
};

/* Gleaned from OFW's set-parameters in cpu/x86/pc/linux.fth */
struct olpc_ofw_header {
	uint32_t ofw_magic;	/* OFW signature */
	uint32_t ofw_version;
	uint32_t cif_handler;	/* callback into OFW */
	uint32_t irq_desc_table;
} __attribute__((packed));

struct efi_info {
	uint32_t efi_loader_signature;
	uint32_t efi_systab;
	uint32_t efi_memdesc_size;
	uint32_t efi_memdesc_version;
	uint32_t efi_memmap;
	uint32_t efi_memmap_size;
	uint32_t efi_systab_hi;
	uint32_t efi_memmap_hi;
};

/*
 * This is the maximum number of entries in struct boot_params::e820_table
 * (the zeropage), which is part of the x86 boot protocol ABI:
 */
#define E820_MAX_ENTRIES_ZEROPAGE 128

/*
 * The E820 memory region entry of the boot protocol ABI:
 */
struct boot_e820_entry {
	uint64_t addr;
	uint64_t size;
	uint32_t type;
} __attribute__((packed));

/* The so-called "zeropage" */
struct boot_params {
	struct screen_info screen_info;			/* 0x000 */
	struct apm_bios_info apm_bios_info;		/* 0x040 */
	uint8_t  _pad2[4];					/* 0x054 */
	uint64_t  tboot_addr;				/* 0x058 */
	struct ist_info ist_info;			/* 0x060 */
	uint64_t acpi_rsdp_addr;				/* 0x070 */
	uint8_t  _pad3[8];					/* 0x078 */
	uint8_t  hd0_info[16];	/* obsolete! */		/* 0x080 */
	uint8_t  hd1_info[16];	/* obsolete! */		/* 0x090 */
	struct sys_desc_table sys_desc_table; /* obsolete! */	/* 0x0a0 */
	struct olpc_ofw_header olpc_ofw_header;		/* 0x0b0 */
	uint32_t ext_ramdisk_image;			/* 0x0c0 */
	uint32_t ext_ramdisk_size;				/* 0x0c4 */
	uint32_t ext_cmd_line_ptr;				/* 0x0c8 */
	uint8_t  _pad4[116];				/* 0x0cc */
	struct edid_info edid_info;			/* 0x140 */
	struct efi_info efi_info;			/* 0x1c0 */
	uint32_t alt_mem_k;				/* 0x1e0 */
	uint32_t scratch;		/* Scratch field! */	/* 0x1e4 */
	uint8_t  e820_entries;				/* 0x1e8 */
	uint8_t  eddbuf_entries;				/* 0x1e9 */
	uint8_t  edd_mbr_sig_buf_entries;			/* 0x1ea */
	uint8_t  kbd_status;				/* 0x1eb */
	uint8_t  secure_boot;				/* 0x1ec */
	uint8_t  _pad5[2];					/* 0x1ed */
	/*
	 * The sentinel is set to a nonzero value (0xff) in header.S.
	 *
	 * A bootloader is supposed to only take setup_header and put
	 * it into a clean boot_params buffer. If it turns out that
	 * it is clumsy or too generous with the buffer, it most
	 * probably will pick up the sentinel variable too. The fact
	 * that this variable then is still 0xff will let kernel
	 * know that some variables in boot_params are invalid and
	 * kernel should zero out certain portions of boot_params.
	 */
	uint8_t  sentinel;					/* 0x1ef */
	uint8_t  _pad6[1];					/* 0x1f0 */
	struct setup_header hdr;    /* setup header */	/* 0x1f1 */
	uint8_t  _pad7[0x290-0x1f1-sizeof(struct setup_header)];
	uint32_t edd_mbr_sig_buffer[EDD_MBR_SIG_MAX];	/* 0x290 */
	struct boot_e820_entry e820_table[E820_MAX_ENTRIES_ZEROPAGE]; /* 0x2d0 */
	uint8_t  _pad8[48];				/* 0xcd0 */
	struct edd_info eddbuf[EDDMAXNR];		/* 0xd00 */
	uint8_t  _pad9[276];				/* 0xeec */
} __attribute__((packed));
/* asm/bootparam.h */


void LinuxLoad(boot_entry_s* entry)
{
	FILE* kernelFile = fopen(entry->imgToLoad, "r");
	if (kernelFile == NULL)
	{
		printf("linux: Failed to open kernel file `%s`.\n", entry->imgToLoad);
		return;
	}

	uint32_t hdrS = 0;
	fseek(kernelFile, 0x202, SEEK_SET);
	fread(&hdrS, sizeof(uint32_t), 1, kernelFile);
	if (hdrS != 0x53726448)
	{
		printf("linux: Old boot protocol version.\n");
		goto cleanup_early;
	}

	size_t setupCodeSize = 0;
	size_t realModeCodeSize = 0;
	fseek(kernelFile, 0x1f1, SEEK_SET);
	fread(&setupCodeSize, 1, 1, kernelFile);
	if (setupCodeSize == 0)
	{
		setupCodeSize = 4;
	}
	setupCodeSize *= 512;
	realModeCodeSize = 512 + setupCodeSize;

	struct boot_params* bootParams = calloc(1, sizeof(struct boot_params));
	if (bootParams == NULL)
	{
		printf("linux: Failed to allocate memory for bootParams.\n");
		goto cleanup_early;
	}
	struct setup_header* setupHeader = &bootParams->hdr;

	size_t setupHeaderEnd = 0;
	{
		uint8_t temp = 0;
		fseek(kernelFile, 0x201, SEEK_SET);
		fread(&temp, 1, 1, kernelFile);
		setupHeaderEnd = temp + 0x202;
	}

	fseek(kernelFile, 0x1f1, SEEK_SET);
	fread(setupHeader, 1, setupHeaderEnd - 0x1f1, kernelFile);
	if (setupHeader->version < 0x203)
	{
		printf("linux: Boot protocols of versions older than 2.03 are not supported.\n");
		goto cleanup_early;
	}

	if (!(setupHeader->loadflags & (1 << 0)))
	{
		printf("linux: Kernels that load at 0x10000 are not supported.\n");
		goto cleanup_early;
	}

	setupHeader->cmd_line_ptr = (uint32_t)(uintptr_t)entry->imgArgs;
	setupHeader->vid_mode = 0xFFFF; // "normal" mode
	setupHeader->type_of_loader = 0xFF;
	setupHeader->loadflags &= ~(1 << 5); // Print early messages

	fseek(kernelFile, 0, SEEK_END);
	size_t kernelFileSize = ftell(kernelFile);
	uintn_t kernelSizeInPages = AlignUp(kernelFileSize, 4096) / 4096;

	/*
	*	Loading the kernel
	*/
	uintptr_t kernelLoadAddr = 0x100000;
	while (1)
	{
		// Attempt to allocate memory at 0x100000, if that fails, increase the base pointer
		efi_status_t status = BS->AllocatePages(AllocateAddress, EfiLoaderData, 
			kernelSizeInPages, (efi_physical_address_t*)&kernelLoadAddr);
		if (!EFI_ERROR(status))
		{
			break;
		}
		kernelLoadAddr += 0x100000;
	}
	// Load the kernel into memory
	fseek(kernelFile, realModeCodeSize, SEEK_SET);
	fread((void*)kernelLoadAddr, 1, kernelFileSize - realModeCodeSize, kernelFile);

	fclose(kernelFile);
	kernelFile = NULL;

	/*
	*	Loading the initrds
	*/
	uint32_t initrdBaseAddr = setupHeader->initrd_addr_max;
	if (initrdBaseAddr == 0)
	{
		initrdBaseAddr = 0x38000000;
	}
	size_t sizeOfAllInitrds = 0;
	
	// Get the total size of all initrds
	for (uint32_t i = 0; i < entry->initrdAmount; i++)
	{
		FILE* initrdFP = fopen(entry->initrdPaths[i], "r");
		if (initrdFP == NULL)
		{
			printf("linux: Failed to open initrd `%s`: %s\n", entry->initrdPaths[i], 
				GetCommandErrorInfo(errno));
			goto cleanup_kernel;
		}
		
		fseek(initrdFP, 0, SEEK_END);
		sizeOfAllInitrds += ftell(initrdFP);

		fclose(initrdFP);
	}

	initrdBaseAddr -= sizeOfAllInitrds;
	initrdBaseAddr = AlignDown(initrdBaseAddr, 4096);
	uintn_t initrdSizeInPages = AlignUp(sizeOfAllInitrds, 4096) / 4096;

	while (1)
	{
		// Casting initrdBaseAddr does not work so we store it in a temporary 64bit var
		uint64_t initrdBaseAddr64Bit = initrdBaseAddr;
		// Attempt to allocate memory for the initrds, if that fails, decrease the base pointer
		efi_status_t status = BS->AllocatePages(AllocateAddress, EfiLoaderData, 
			initrdSizeInPages, (efi_physical_address_t*)&initrdBaseAddr64Bit);
		if (!EFI_ERROR(status))
		{
			break;
		}
		initrdBaseAddr -= 4096;
	}

	size_t _tempInitrdBaseAddr = initrdBaseAddr;
	for (uint32_t i = 0; i < entry->initrdAmount; i++)
	{
		FILE* initrdFP = fopen(entry->initrdPaths[i], "r");
		if (initrdFP == NULL)
		{
			printf("linux: Failed to open initrd `%s`: %s\n", entry->initrdPaths[i], 
				GetCommandErrorInfo(errno));
			goto cleanup_initrds;
		}

		// Get the size of the initrd
		fseek(initrdFP, 0, SEEK_END);
		size_t initrdSize = ftell(initrdFP);
		fseek(initrdFP, 0, SEEK_SET);

		// Read the data of the initrd into memory
		fread((void*)_tempInitrdBaseAddr, 1, initrdSize, initrdFP);
		_tempInitrdBaseAddr += initrdSize;

		fclose(initrdFP);
	}

	if (sizeOfAllInitrds != 0)
	{
		setupHeader->ramdisk_image = (uint32_t)initrdBaseAddr;
		setupHeader->ramdisk_size = (uint32_t)sizeOfAllInitrds;
	}

	/*
	*	Video
	*/
	struct screen_info* screenInfo = &bootParams->screen_info;
	framebuffer_t* fb = GetFrameBufferInfo();
	if (fb == NULL)
	{
		printf("linux: Failed to get framebuffer info.\n");
		goto cleanup_initrds;
	}
	screenInfo->capabilities		= VIDEO_CAPABILITY_64BIT_BASE | VIDEO_CAPABILITY_SKIP_QUIRKS;
	screenInfo->flags				= VIDEO_FLAGS_NOCURSOR;
	screenInfo->lfb_base			= (uint32_t)fb->framebuffer_addr;
	screenInfo->ext_lfb_base		= (uint32_t)(fb->framebuffer_addr >> 32);
	screenInfo->lfb_size			= fb->framebuffer_pitch * fb->framebuffer_height;
	screenInfo->lfb_width			= fb->framebuffer_width;
	screenInfo->lfb_height			= fb->framebuffer_height;
	screenInfo->lfb_depth			= fb->framebuffer_bpp;
	screenInfo->lfb_linelength		= fb->framebuffer_pitch;
	screenInfo->red_size			= fb->red_mask_size;
	screenInfo->red_pos				= fb->red_mask_shift;
	screenInfo->green_size			= fb->green_mask_size;
	screenInfo->green_pos			= fb->green_mask_shift;
	screenInfo->blue_size			= fb->blue_mask_size;
	screenInfo->blue_pos			= fb->blue_mask_shift;
	screenInfo->orig_video_isVGA	= VIDEO_TYPE_EFI;
	free(fb);

	/*
	*	EDID
	*/
	efi_edid_active_protocol_t* edid = GetEdidActiveProtocol();
	if (edid != NULL)
	{
		memcpy(&bootParams->edid_info, edid->Edid, sizeof(struct edid_info));
	}
	
	/*
	*	RSDP
	*/
	bootParams->acpi_rsdp_addr = (uint64_t)GetRSDP();

	/*
	*	UEFI
	*/
	uintn_t mmapSize = 0;
	efi_memory_descriptor_t* efimmap = NULL;
	uintn_t mmapDescSize = 0;
	uint32_t mmapDescVer = 0;
	uintn_t mmapKey = 0;
	efi_status_t status = BS->GetMemoryMap(&mmapSize, efimmap, &mmapKey, &mmapDescSize, &mmapDescVer);
	if (status != EFI_BUFFER_TOO_SMALL)
	{
		printf("linux: Failed to get the memory map size. (error %d)", status ^ EFI_ERROR_MASK);
		goto cleanup_initrds;
	}
	efimmap = malloc(mmapSize);
	if (efimmap == NULL)
	{
		printf("linux: Failed to allocate memory for the memory map.\n");
		goto cleanup_initrds;
	}
	status = BS->GetMemoryMap(&mmapSize, efimmap, &mmapKey, &mmapDescSize, &mmapDescVer);
	if (EFI_ERROR(status))
	{
		printf("linux: Failed to get the memory map. (error %d)", status ^ EFI_ERROR_MASK);
		goto cleanup_memmap;
	}

	memcpy(&bootParams->efi_info.efi_loader_signature, "EL64", 4);
	bootParams->efi_info.efi_systab    = (uint32_t)(uint64_t)(uintptr_t)ST;
	bootParams->efi_info.efi_systab_hi = (uint32_t)((uint64_t)(uintptr_t)ST >> 32);
	bootParams->efi_info.efi_memmap    = (uint32_t)(uint64_t)(uintptr_t)efimmap;
	bootParams->efi_info.efi_memmap_hi = (uint32_t)((uint64_t)(uintptr_t)efimmap >> 32);
	bootParams->efi_info.efi_memmap_size     = mmapSize;
	bootParams->efi_info.efi_memdesc_size    = mmapDescSize;
	bootParams->efi_info.efi_memdesc_version = mmapDescVer;
	exit_bs();

// Cleanup in stages
cleanup_memmap:
	free(efimmap);
cleanup_initrds:
	BS->FreePages((efi_physical_address_t)initrdBaseAddr, initrdSizeInPages);
cleanup_kernel:
	BS->FreePages((efi_physical_address_t)kernelLoadAddr, kernelSizeInPages);
cleanup_early:
	fclose(kernelFile);
}
