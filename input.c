#include "input.h"
#include "uinput.h"
#include "rce.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/timerfd.h>
#include <unistd.h>

struct input_state_t {
    bool pressing;
    int pressed_device_id;
    int pos_x, pos_y;
    int pressed_pos_x, pressed_pos_y;
    int fd_timer; // The timer used to execute right click on timeout
    struct timespec start;
    struct libevdev_uinput *uinput;
};

int fd_is_valid(int fd) {
    struct stat s;
    fstat(fd, &s);
    return s.st_nlink > 0;
}

void free_evdev(struct libevdev *evdev) {
    int fd = libevdev_get_fd(evdev);
    libevdev_free(evdev);
    close(fd);
}

// Build the fd set used by `select`
// return: nfds to be used in select()
int build_fd_set(fd_set *fds, int fd_timer,
        int num, struct libevdev **evdev) {
    FD_ZERO(fds);
    FD_SET(fd_timer, fds);
    int fd = -1, max_fd = fd_timer;
    for (unsigned int i = 0; i < num; i++) {
        fd = libevdev_get_fd(evdev[i]);
        FD_SET(fd, fds);
        if (fd > max_fd)
            max_fd = fd;
    }
    return max_fd + 1;
}

void arm_delayed_rclick(struct input_state_t *state, int dev_id) {
    // Start the timer; once timeout reached,
    // fire the right click event
    timerfd_settime(state->fd_timer, 0, &(struct itimerspec) {
        .it_value =
        {
            .tv_sec = 0,
            .tv_nsec = 20 * 1000000,
        },
    }, NULL);
}

void unarm_delayed_rclick(struct input_state_t *state) {
    state->pressed_device_id = -1;
    // Force cancel the timer if finger released
    timerfd_settime(state->fd_timer, 0, &(struct itimerspec) {
        .it_value = {
            .tv_sec = 0,
            .tv_nsec = 0,
        },
    }, NULL);
}

uint64_t millis(struct timespec time) {
   return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

void on_input_event(struct input_state_t *state,
        struct input_event *ev, int dev_id) {
    // If we are during a long-press event,
    // do not interruput with another device
    if (state->pressed_device_id != -1 && dev_id != state->pressed_device_id)
        return;

    if (ev->type == EV_ABS) {
        // Absolute position event
        // Record the current position
        if (ev->code == ABS_X || ev->code == ABS_MT_POSITION_X) {
            state->pos_x = ev->value;
        } else if (ev->code == ABS_Y || ev->code == ABS_MT_POSITION_Y) {
            state->pos_y = ev->value;
        }
    } else if (ev->type == EV_KEY && ev->code == BTN_TOUCH) {
        // Touch event
        if (ev->value == 1) {
            // Record the position where the touch event began and start time
            clock_gettime(CLOCK_MONOTONIC_RAW, &state->start);
            state->pressed_pos_x = state->pos_x;
            state->pressed_pos_y = state->pos_y;
            state->pressed_device_id = dev_id;
        } else {
            struct timespec end;
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            uint64_t delta_us = millis(end) - millis(state->start);
            if (delta_us >= millis(LONG_CLICK_INTERVAL)) {
                unsigned int dx = abs(state->pos_x - state->pressed_pos_x);
                unsigned int dy = abs(state->pos_y - state->pressed_pos_y);
                // Only consider movement within a range to be "still"
                // i.e. if movement is within this value during timeout,
                //   then it is a long click
                if (dx <= LONG_CLICK_FUZZ && dy <= LONG_CLICK_FUZZ) {
                   arm_delayed_rclick(state, dev_id);
                }
            }
        }
    }
}

void on_timer_expire(struct input_state_t *state) {
    uinput_send_right_click_down(state->uinput);
    uinput_send_right_click_up(state->uinput);
    state->pressing = true;
    // In Linux implementation of timerfd, the fd becomes always "readable"
    // after the timeout. So we have to unarm it after we receive the event.
    unarm_delayed_rclick(state);
}

void process_evdev_input(int num, struct libevdev **evdev) {
    // Initialize everything to -1
    struct input_state_t state = {
        .pressing = false,
        .pressed_device_id = -1,
        .pos_x = -1, .pos_y = -1,
        .pressed_pos_x = -1, .pressed_pos_y = -1,
        .fd_timer = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK),
        .uinput = uinput_initialize(),
    };

    // Check if uinput device was created
    if (state.uinput == NULL) {
        fprintf(stderr, "Failed to create uinput device\n");
        exit(-1);
        return;
    }

    // Initialize the set of fds
    fd_set fds;
    int nfds = build_fd_set(&fds, state.fd_timer, num, evdev);

    struct input_event ev;
    int fd_valid = 1;
    struct timespec last;
    clock_gettime(CLOCK_MONOTONIC_RAW, &last);
    // Detect events from the fds
    while (select(nfds, &fds, NULL, NULL, NULL) > 0) {

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        uint64_t delta_us = millis(now) - millis(last);
        
        // The timer fd
        if (FD_ISSET(state.fd_timer, &fds))
            on_timer_expire(&state);

        for (unsigned int i = 0; i < num; i++) {
            int fd = libevdev_get_fd(evdev[i]);
            if (delta_us >= 5000) {
                clock_gettime(CLOCK_MONOTONIC_RAW, &last);
                if (!fd_is_valid(fd)) {
                    fd_valid = 0;
                    sleep(5);
                    break;
                }
            }
            // Can we read an event?
            if (!FD_ISSET(fd, &fds))
                continue;
            
            // Read all the available events
            while (libevdev_next_event(evdev[i],
                    LIBEVDEV_READ_FLAG_NORMAL, &ev) == 0) {
                on_input_event(&state, &ev, i);
            }
            fd_valid = 1;
        }
        if (!fd_valid) {
            break;
        }

        // Reset the fd_set for next iteration
        nfds = build_fd_set(&fds, state.fd_timer, num, evdev);
    }

    // Cleanup
    close(state.fd_timer);
    libevdev_uinput_destroy(state.uinput);
    for (int i = 0; i < num; i++) {
        free_evdev(evdev[i]);
    }
}
