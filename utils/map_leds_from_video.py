
import cv2
import csv
import os
import numpy as np

VIDEO_PATH = "led_video.mp4"      # Name of your recorded video file
OUTPUT_CSV = "led_coordinates.csv"
LED_COUNT = 329                   # Total number of LEDs
FPS = 30                          # Frames per second of your video
SECONDS_PER_LED = 0.5             # Duration each LED stays on
FRAMES_PER_LED = int(FPS * SECONDS_PER_LED)
MARGIN = 5                        # Skip this many frames at start/end of each window

BRIGHTNESS_THRESHOLD = 220
MIN_BLOB_AREA = 20
DEBUG_VISUALS = True

def find_brightest_centroid(frame, threshold=200, min_blob_area=20):

    debug_frame = None
    if DEBUG_VISUALS:
        debug_frame = frame.copy()

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    _, thresh = cv2.threshold(gray, threshold, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    max_area = 0
    best_centroid = None

    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area >= min_blob_area and area > max_area:
            M = cv2.moments(cnt)
            if M["m00"] != 0:
                cx = int(M["m10"] / M["m00"])
                cy = int(M["m01"] / M["m00"])
                best_centroid = (cx, cy)
                max_area = area
                if DEBUG_VISUALS:
                    cv2.drawContours(debug_frame, [cnt], -1, (0, 255, 0), 2)
                    cv2.circle(debug_frame, (cx, cy), 4, (0, 0, 255), -1)

    if DEBUG_VISUALS:
        cv2.imshow("Thresholded + Contours", debug_frame)
        cv2.waitKey(1)

    return best_centroid  # None if no valid blob found


cap = cv2.VideoCapture(VIDEO_PATH)

if not cap.isOpened():
    print("Error opening video file")
    exit(1)

frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
print(f"Total frames: {frame_count}")

led_coords = []

for led_index in range(LED_COUNT):
    print(f"Mapping LED {led_index}...")
    start_frame = led_index * FRAMES_PER_LED + MARGIN
    end_frame = (led_index + 1) * FRAMES_PER_LED - MARGIN
    centroids = []

    for frame_num in range(start_frame, end_frame):
        cap.set(cv2.CAP_PROP_POS_FRAMES, frame_num)
        ret, frame = cap.read()
        if not ret:
            continue

        centroid = find_brightest_centroid(frame, BRIGHTNESS_THRESHOLD, MIN_BLOB_AREA)
        if centroid:
            centroids.append(centroid)

    # Reject outliers using distance-based filtering
    if centroids:
        arr = np.array(centroids)
        mean = np.mean(arr, axis=0)
        dists = np.linalg.norm(arr - mean, axis=1)
        filtered = arr[dists <= np.std(dists) * 1.5]

        if len(filtered) > 0:
            avg_centroid = np.mean(filtered, axis=0).astype(int)
            led_coords.append((led_index, avg_centroid[0], avg_centroid[1]))
        else:
            led_coords.append((led_index, -1, -1))
            print("unstable")
    else:
        led_coords.append((led_index, -1, -1))
        print("notfound")

cap.release()
cv2.destroyAllWindows()

# === SAVE TO CSV ===
with open(OUTPUT_CSV, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["LED_index", "x", "y"])
    for row in led_coords:
        writer.writerow(row)

print(f"Mapping complete. Saved to {OUTPUT_CSV}")
