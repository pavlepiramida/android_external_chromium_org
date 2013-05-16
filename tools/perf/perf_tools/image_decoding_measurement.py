# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from telemetry.page import page_measurement


class ImageDecoding(page_measurement.PageMeasurement):
  def WillNavigateToPage(self, page, tab):
    tab.StartTimelineRecording()

  def MeasurePage(self, page, tab, results):
    tab.StopTimelineRecording()
    def _IsDone():
      return tab.EvaluateJavaScript('isDone')

    decode_image_events = \
        tab.timeline_model.GetAllOfName('DecodeImage')

    # If it is a real image page, then store only the last-minIterations
    # decode tasks.
    if (hasattr(page,
               'image_decoding_measurement_limit_results_to_min_iterations') and
        page.image_decoding_measurement_limit_results_to_min_iterations):
      assert _IsDone()
      min_iterations = tab.EvaluateJavaScript('minIterations')
      decode_image_events = decode_image_events[-min_iterations:]

    durations = [d.duration for d in decode_image_events]
    if not durations:
      results.Add('ImageDecoding_avg', 'ms', 'unsupported')
      return
    image_decoding_avg = sum(durations) / len(durations)
    results.Add('ImageDecoding_avg', 'ms', image_decoding_avg)
