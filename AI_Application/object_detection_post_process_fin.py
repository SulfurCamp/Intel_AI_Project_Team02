import cv2
import numpy as np
from common.toolbox import id_to_color


def inference_result_handler(original_frame, infer_results, labels, config_data, tracker=None, return_detection=False):
    """
    return_detection=True  : 픽셀 좌표의 검출 리스트(list[dict])만 반환 (그리기 X)
    return_detection=False : 기존대로 프레임 위에 박스/트래커를 그려서 반환
    """
    H, W = original_frame.shape[:2]

    # 1) 기존 디코딩(프로젝트 원래 로직) → dict 형태 기대
    det_raw = extract_detections(original_frame, infer_results, config_data)

    if return_detection:
        det_list = []

        # --- dict 케이스(권장) ---
        if isinstance(det_raw, dict):
            # 다양한 키 이름 지원
            boxes   = (det_raw.get("detection_boxes")
                       or det_raw.get("boxes") or det_raw.get("bboxes") or det_raw.get("bbox"))
            classes = (det_raw.get("detection_classes")
                       or det_raw.get("classes") or det_raw.get("class_ids") or det_raw.get("labels"))
            scores  = (det_raw.get("detection_scores")
                       or det_raw.get("scores") or det_raw.get("conf") or det_raw.get("confidences"))

            if boxes is None:
                return []

            boxes = np.asarray(boxes)
            if boxes.ndim >= 2 and boxes.shape[-1] >= 4:
                boxes = boxes.reshape(-1, boxes.shape[-1])[:, :4]
            else:
                return []

            N = boxes.shape[0]
            classes = np.zeros((N,), dtype=int)   if classes is None else np.asarray(classes).reshape(-1)[:N]
            scores  = np.ones((N,), dtype=float)  if scores  is None else np.asarray(scores).reshape(-1)[:N]

            # 0~1로 보이면 픽셀로 변환
            norm_like = (boxes.min() >= -0.05) and (boxes.max() <= 1.2)
            if norm_like:
                x1 = np.clip((boxes[:,0] * W).round().astype(int), 0, W-1)
                y1 = np.clip((boxes[:,1] * H).round().astype(int), 0, H-1)
                x2 = np.clip((boxes[:,2] * W).round().astype(int), 0, W-1)
                y2 = np.clip((boxes[:,3] * H).round().astype(int), 0, H-1)
            else:
                x1 = np.clip(boxes[:,0].round().astype(int), 0, W-1)
                y1 = np.clip(boxes[:,1].round().astype(int), 0, H-1)
                x2 = np.clip(boxes[:,2].round().astype(int), 0, W-1)
                y2 = np.clip(boxes[:,3].round().astype(int), 0, H-1)

            w = x2 - x1; h = y2 - y1
            keep = (w > 1) & (h > 1) & (scores > 0.0)

            for xx1, yy1, xx2, yy2, sc, ci in zip(x1[keep], y1[keep], x2[keep], y2[keep], scores[keep], classes[keep]):
                det_list.append({
                    "class_id": int(ci),
                    "accuracy": float(sc),   # ★ 요청: accuracy 포함
                    "score": float(sc),      # (호환용) score도 유지
                    "bbox": {"x1": int(xx1), "y1": int(yy1), "x2": int(xx2), "y2": int(yy2)}
                })
            return det_list

        # --- list[dict] 케이스(다른 포맷 방어) ---
        if isinstance(det_raw, list):
            for d in det_raw:
                if not isinstance(d, dict):
                    continue
                if "bbox" in d:
                    b = d["bbox"]
                    x1, y1, x2, y2 = b.get("x1"), b.get("y1"), b.get("x2"), b.get("y2")
                    if None in (x1, y1, x2, y2):
                        continue
                    sc = float(d.get("accuracy", d.get("score", 0.0)))  # accuracy 우선
                    det_list.append({
                        "class_id": int(d.get("class_id", 0)),
                        "accuracy": sc,
                        "score": sc,
                        "bbox": {
                            "x1": int(round(x1)), "y1": int(round(y1)),
                            "x2": int(round(x2)), "y2": int(round(y2)),
                        }
                    })
            return det_list

        # 알 수 없는 포맷
        return []

    # === return_detection=False → 화면에 그려서 반환 ===
    frame_with = draw_detections(det_raw, original_frame, labels, tracker=tracker)
    return frame_with



def draw_detection(image: np.ndarray, box: list, labels: list, score: float, color: tuple, track=False):
    """
    Draw box and label for one detection.

    Args:
        image (np.ndarray): Image to draw on.
        box (list): Bounding box coordinates.
        labels (list): List of labels (1 or 2 elements).
        score (float): Detection score.
        color (tuple): Color for the bounding box.
        track (bool): Whether to include tracking info.
    """
    ymin, xmin, ymax, xmax = map(int, box)
    cv2.rectangle(image, (xmin, ymin), (xmax, ymax), color, 2)
    font = cv2.FONT_HERSHEY_SIMPLEX

    # Compose texts
    top_text = f"{labels[0]}: {score:.1f}%" if not track or len(labels) == 2 else f"{score:.1f}%"
    bottom_text = None

    if track:
        if len(labels) == 2:
            bottom_text = labels[1]
        else:
            bottom_text = labels[0]


    # Set colors
    text_color = (255, 255, 255)  # white
    border_color = (0, 0, 0)      # black

    # Draw top text with black border first
    cv2.putText(image, top_text, (xmin + 4, ymin + 20), font, 0.5, border_color, 2, cv2.LINE_AA)
    cv2.putText(image, top_text, (xmin + 4, ymin + 20), font, 0.5, text_color, 1, cv2.LINE_AA)

    # Draw bottom text if exists
    if bottom_text:
        pos = (xmax - 50, ymax - 6)
        cv2.putText(image, bottom_text, pos, font, 0.5, border_color, 2, cv2.LINE_AA)
        cv2.putText(image, bottom_text, pos, font, 0.5, text_color, 1, cv2.LINE_AA)


def denormalize_and_rm_pad(box: list, size: int, padding_length: int, input_height: int, input_width: int) -> list:
    """
    Denormalize bounding box coordinates and remove padding.

    Args:
        box (list): Normalized bounding box coordinates.
        size (int): Size to scale the coordinates.
        padding_length (int): Length of padding to remove.
        input_height (int): Height of the input image.
        input_width (int): Width of the input image.

    Returns:
        list: Denormalized bounding box coordinates with padding removed.
    """
    for i, x in enumerate(box):
        box[i] = int(x * size)
        if (input_width != size) and (i % 2 != 0):
            box[i] -= padding_length
        if (input_height != size) and (i % 2 == 0):
            box[i] -= padding_length

    return box


def extract_detections(image: np.ndarray, detections: list, config_data) -> dict:
    """
    Extract detections from the input data.

    Args:
        image (np.ndarray): Image to draw on.
        detections (list): Raw detections from the model.
        config_data (Dict): Loaded JSON config containing post-processing metadata.

    Returns:
        dict: Filtered detection results containing 'detection_boxes', 'detection_classes', 'detection_scores', and 'num_detections'.
    """

    visualization_params = config_data["visualization_params"]
    score_threshold = visualization_params.get("score_thres", 0.5)
    max_boxes = visualization_params.get("max_boxes_to_draw", 50)


    #values used for scaling coords and removing padding
    img_height, img_width = image.shape[:2]
    size = max(img_height, img_width)
    padding_length = int(abs(img_height - img_width) / 2)

    all_detections = []

    for class_id, detection in enumerate(detections):
        for det in detection:
            bbox, score = det[:4], det[4]
            if score >= score_threshold:
                denorm_bbox = denormalize_and_rm_pad(bbox, size, padding_length, img_height, img_width)
                all_detections.append((score, class_id, denorm_bbox))

    #sort all detections by score descending
    all_detections.sort(reverse=True, key=lambda x: x[0])

    #take top max_boxes
    top_detections = all_detections[:max_boxes]

    scores, class_ids, boxes = zip(*top_detections) if top_detections else ([], [], [])

    return {
        'detection_boxes': list(boxes),
        'detection_classes': list(class_ids),
        'detection_scores': list(scores),
        'num_detections': len(top_detections)
    }


def draw_detections(detections: dict, img_out: np.ndarray, labels, tracker=None):
    """
    Draw detections or tracking results on the image.

    Args:
        detections (dict): Raw detection outputs.
        img_out (np.ndarray): Image to draw on.
        labels (list): List of class labels.
        enable_tracking (bool): Whether to use tracker output (ByteTrack).
        tracker (BYTETracker, optional): ByteTrack tracker instance.

    Returns:
        np.ndarray: Annotated image.
    """

    #extract detection data from the dictionary
    boxes = detections["detection_boxes"]  # List of [xmin,ymin,xmaxm, ymax] boxes
    scores = detections["detection_scores"]  # List of detection confidences
    num_detections = detections["num_detections"]  # Total number of valid detections
    classes = detections["detection_classes"]  # List of class indices per detection

    if tracker:
        dets_for_tracker = []

        #Convert detection format to [xmin,ymin,xmaxm ymax,score] for tracker
        for idx in range(num_detections):
            box = boxes[idx]  #[x, y, w, h]
            score = scores[idx]
            dets_for_tracker.append([*box, score])

        #skip tracking if no detections passed
        if not dets_for_tracker:
            return img_out

        #run BYTETracker and get active tracks
        online_targets = tracker.update(np.array(dets_for_tracker))

        #draw tracked bounding boxes with ID labels
        for track in online_targets:
            track_id = track.track_id  #unique tracker ID
            x1, y1, x2, y2 = track.tlbr  #bounding box (top-left, bottom-right)
            xmin, ymin, xmax, ymax = map(int, [x1, y1, x2, y2])
            best_idx = find_best_matching_detection_index(track.tlbr, boxes)
            color = tuple(id_to_color(classes[best_idx]).tolist())  # color based on class
            if best_idx is None:
                draw_detection(img_out, [xmin, ymin, xmax, ymax], f"ID {track_id}",
                               track.score * 100.0, color, track=True)
            else:
                draw_detection(img_out, [xmin, ymin, xmax, ymax], [labels[classes[best_idx]], f"ID {track_id}"],
                               track.score * 100.0, color, track=True)



    else:
        #No tracking — draw raw model detections
        for idx in range(num_detections):
            color = tuple(id_to_color(classes[idx]).tolist())  #color based on class
            draw_detection(img_out, boxes[idx], [labels[classes[idx]]], scores[idx] * 100.0, color)

    return img_out


def find_best_matching_detection_index(track_box, detection_boxes):
    """
    Finds the index of the detection box with the highest IoU relative to the given tracking box.

    Args:
        track_box (list or tuple): The tracking box in [x_min, y_min, x_max, y_max] format.
        detection_boxes (list): List of detection boxes in [x_min, y_min, x_max, y_max] format.

    Returns:
        int or None: Index of the best matching detection, or None if no match is found.
    """
    best_iou = 0
    best_idx = -1

    for i, det_box in enumerate(detection_boxes):
        iou = compute_iou(track_box, det_box)
        if iou > best_iou:
            best_iou = iou
            best_idx = i

    return best_idx if best_idx != -1 else None


def compute_iou(boxA, boxB):
    """
    Compute Intersection over Union (IoU) between two bounding boxes.

    IoU measures the overlap between two boxes:
        IoU = (area of intersection) / (area of union)
    Values range from 0 (no overlap) to 1 (perfect overlap).

    Args:
        boxA (list or tuple): [x_min, y_min, x_max, y_max]
        boxB (list or tuple): [x_min, y_min, x_max, y_max]

    Returns:
        float: IoU value between 0 and 1.
    """
    xA, yA = max(boxA[0], boxB[0]), max(boxA[1], boxB[1])
    xB, yB = min(boxA[2], boxB[2]), min(boxA[3], boxB[3])
    inter = max(0, xB - xA) * max(0, yB - yA)
    areaA = max(1e-5, (boxA[2] - boxA[0]) * (boxA[3] - boxA[1]))
    areaB = max(1e-5, (boxB[2] - boxB[0]) * (boxB[3] - boxB[1]))
    return inter / (areaA + areaB - inter + 1e-5)
