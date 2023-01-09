/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/BubbleSort.h"

void bubble_sort(double array[], int count) {
    for (int i = count - 1; i >= 0; i--) {
        for (int j = i; j >= 0; j--) {
            if (array[j] < array[j - 1]) {
                double tmp = array[j];
                array[j] = array[j - 1];
                array[j - 1] = tmp;
            }
        }
    }
}
