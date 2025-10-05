#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <signal.h>
#include <wiringPi.h>
#include "epaper/epaper.h"

#define FB_DEVICE "/dev/fb0"
#define UPDATE_INTERVAL 2 

static uint8_t display_buffer[15000];
static int running = 1;

void signal_handler(int sig)
{
    running = 0;
}

void init_display(void)
{
    printf("Initializing E-Ink display...\n");
    epd_set_panel(EPD420, 400, 300);
    epd_io_init();
    printf("Enabling FAST mode...\n");
    epd_init_fast();
    epd_paint_newimage(display_buffer, EPD_W, EPD_H, EPD_ROTATE_270, EPD_COLOR_WHITE);
    printf("Display initialized: %dx%d, fast mode enabled\n", EPD_W, EPD_H);
}

void update_from_framebuffer(int fb_fd)
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fb_data;
    long screensize;
    
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) {
        perror("Error reading fixed info");
        return;
    }
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable info");
        return;
    }
    
    screensize = vinfo.yres_virtual * finfo.line_length;
    fb_data = (char*)mmap(0, screensize, PROT_READ, MAP_SHARED, fb_fd, 0);
    
    if ((void*)fb_data == MAP_FAILED) {
        perror("Error mapping framebuffer");
        return;
    }
    
   
    epd_paint_selectimage(display_buffer);
    epd_paint_clear(EPD_COLOR_WHITE);
    
  
    for (int y = 0; y < EPD_H && y < vinfo.yres; y++) {
        for (int x = 0; x < EPD_W && x < vinfo.xres; x++) {
            int fb_offset = (y * finfo.line_length) + (x * vinfo.bits_per_pixel / 8);
            unsigned char r, g, b;
            
 
            if (vinfo.bits_per_pixel == 32) {

                b = fb_data[fb_offset];
                g = fb_data[fb_offset + 1];
                r = fb_data[fb_offset + 2];
            } else if (vinfo.bits_per_pixel == 24) {
            
                b = fb_data[fb_offset];
                g = fb_data[fb_offset + 1];
                r = fb_data[fb_offset + 2];
            } else if (vinfo.bits_per_pixel == 16) {
           
                unsigned short pixel = *((unsigned short*)(fb_data + fb_offset));
                r = (pixel >> 11) & 0x1F; r = (r << 3) | (r >> 2);
                g = (pixel >> 5) & 0x3F; g = (g << 2) | (g >> 4);
                b = pixel & 0x1F; b = (b << 3) | (b >> 2);
            } else {
          
                r = g = b = fb_data[fb_offset];
            }
            
         
            int luminance = (r + g + b) / 3;
            
        
            if (luminance > 128) {
                epd_paint_setpixel(x, y, EPD_COLOR_BLACK);
            }
       
        }
    }
    
   
    printf("Updating display with fast/partial mode...\n");
    epd_displayBW_partial(display_buffer);
    
    munmap(fb_data, screensize);
}


void update_from_framebuffer_advanced(int fb_fd)
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fb_data;
    long screensize;
    
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) {
        perror("Error reading fixed info");
        return;
    }
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable info");
        return;
    }
    
    screensize = vinfo.yres_virtual * finfo.line_length;
    fb_data = (char*)mmap(0, screensize, PROT_READ, MAP_SHARED, fb_fd, 0);
    
    if ((void*)fb_data == MAP_FAILED) {
        perror("Error mapping framebuffer");
        return;
    }
    
    epd_paint_selectimage(display_buffer);
    epd_paint_clear(EPD_COLOR_WHITE);
    

    for (int y = 0; y < EPD_H && y < vinfo.yres; y++) {
        for (int x = 0; x < EPD_W && x < vinfo.xres; x++) {
            int fb_offset = (y * finfo.line_length) + (x * vinfo.bits_per_pixel / 8);
            unsigned char r, g, b;
            
            if (vinfo.bits_per_pixel == 32) {
                b = fb_data[fb_offset];
                g = fb_data[fb_offset + 1];
                r = fb_data[fb_offset + 2];
            } else if (vinfo.bits_per_pixel == 24) {
                b = fb_data[fb_offset];
                g = fb_data[fb_offset + 1];
                r = fb_data[fb_offset + 2];
            } else if (vinfo.bits_per_pixel == 16) {
                unsigned short pixel = *((unsigned short*)(fb_data + fb_offset));
                r = (pixel >> 11) & 0x1F; r = (r << 3) | (r >> 2);
                g = (pixel >> 5) & 0x3F; g = (g << 2) | (g >> 4);
                b = pixel & 0x1F; b = (b << 3) | (b >> 2);
            } else {
                r = g = b = fb_data[fb_offset];
            }
            

            // float luminance = 0.299f * r + 0.587f * g + 0.114f * b;
            int luminance = (r * 299 + g * 587 + b * 114) / 1000;
            

            if (luminance > 64) { 
                epd_paint_setpixel(x, y, EPD_COLOR_BLACK);
            }
        }
    }
    
    printf("Fast display update...\n");
    epd_displayBW_partial(display_buffer);
    
    munmap(fb_data, screensize);
}

int main()
{
    int fb_fd;
    int update_count = 0;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("=== E-Ink Framebuffer Daemon (INVERTED + FAST) ===\n");
    printf("Source: %s\n", FB_DEVICE);
    printf("Update interval: %d seconds\n", UPDATE_INTERVAL);
    printf("Color mode: INVERTED (black on white)\n");
    printf("Update mode: FAST\n");
    

    fb_fd = open(FB_DEVICE, O_RDONLY);
    if (fb_fd == -1) {
        perror("Error opening framebuffer");
        return 1;
    }
    

    init_display();
    
    printf("Started. Press Ctrl+C to exit.\n");
    
    while (running) {
        update_count++;
        printf("Update #%d... ", update_count);
        fflush(stdout);
        

        update_from_framebuffer_advanced(fb_fd);
        
        printf("done.\n");
        sleep(UPDATE_INTERVAL);
    }
    
    printf("Shutting down... wait a second...\n");
    close(fb_fd);
    

    epd_enter_deepsleepmode(EPD_DEEPSLEEP_MODE1);
    epd_io_deinit();
    
    printf("DONE\n");
    return 0;
}
