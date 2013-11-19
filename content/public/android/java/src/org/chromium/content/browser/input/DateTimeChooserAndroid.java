// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser.input;

import android.content.Context;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.content.browser.ContentViewCore;

/**
 * Plumbing for the different date/time dialog adapters.
 */
@JNINamespace("content")
class DateTimeChooserAndroid {

    private final long mNativeDateTimeChooserAndroid;
    private final InputDialogContainer mInputDialogContainer;

    private DateTimeChooserAndroid(Context context,
            long nativeDateTimeChooserAndroid) {
        mNativeDateTimeChooserAndroid = nativeDateTimeChooserAndroid;
        mInputDialogContainer = new InputDialogContainer(context,
                new InputDialogContainer.InputActionDelegate() {

            @Override
            public void replaceDateTime(
                    int dialogType,
                    int year, int month, int day, int hour, int minute,
                    int second, int milli, int week) {
                nativeReplaceDateTime(mNativeDateTimeChooserAndroid,
                        dialogType,
                        year, month, day, hour, minute, second, milli, week);
            }

            @Override
            public void cancelDateTimeDialog() {
                nativeCancelDialog(mNativeDateTimeChooserAndroid);
            }
        });
    }

    private void showDialog(int dialogType, int year, int month, int monthDay,
                            int hour, int minute, int second, int milli,
                            int week, double min, double max, double step) {
        mInputDialogContainer.showDialog(
            dialogType, year, month, monthDay,
            hour, minute, second, milli, week, min, max, step);
    }

    @CalledByNative
    private static DateTimeChooserAndroid createDateTimeChooser(
            ContentViewCore contentViewCore,
            long nativeDateTimeChooserAndroid, int dialogType,
            int year, int month, int day,
            int hour, int minute, int second, int milli, int week,
            double min, double max, double step) {
        DateTimeChooserAndroid chooser =
                new DateTimeChooserAndroid(
                        contentViewCore.getContext(),
                        nativeDateTimeChooserAndroid);
        chooser.showDialog(
            dialogType, year, month, day, hour, minute, second, milli,
            week, min, max, step);
        return chooser;
    }

    @CalledByNative
    private static void initializeDateInputTypes(
            int textInputTypeDate, int textInputTypeDateTime,
            int textInputTypeDateTimeLocal, int textInputTypeMonth,
            int textInputTypeTime, int textInputTypeWeek) {
        InputDialogContainer.initializeInputTypes(textInputTypeDate,
                textInputTypeDateTime, textInputTypeDateTimeLocal,
                textInputTypeMonth, textInputTypeTime, textInputTypeWeek);
    }

    private native void nativeReplaceDateTime(
            long nativeDateTimeChooserAndroid, int dialogType,
            int year, int month, int day, int hour, int minute,
            int second, int milli, int week);

    private native void nativeCancelDialog(long nativeDateTimeChooserAndroid);
}
