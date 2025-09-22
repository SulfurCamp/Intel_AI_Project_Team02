# parse.py

from hailo_sdk_client import ClientRunner

# --- 설정 ---
# 파일 목록의 'best.onnx'를 기준으로 모델 이름을 'best'로 설정합니다.
MODEL_NAME = 'best'
ONNX_MODEL_PATH = f'{MODEL_NAME}.onnx'
PARSED_HAR_PATH = f'{MODEL_NAME}_parsed.har'
HW_ARCH = 'hailo8'

def main():
    """ONNX 모델을 파싱하여 기본 HAR 파일을 생성합니다."""
    print(f"--- 1단계: 파싱 시작 ---")
    print(f"입력 ONNX 파일: {ONNX_MODEL_PATH}")

    runner = ClientRunner(hw_arch=HW_ARCH)
    
    runner.translate_onnx_model(
        ONNX_MODEL_PATH, 
        MODEL_NAME,
        end_node_names=["/model.22/cv2.2/cv2.2.2/Conv", "/model.22/cv3.2/cv3.2.2/Conv",
            "/model.22/cv2.1/cv2.1.2/Conv", "/model.22/cv3.1/cv3.1.2/Conv",
            "/model.22/cv2.0/cv2.0.2/Conv", "/model.22/cv3.0/cv3.0.2/Conv"],
        net_input_shapes={"images": [1, 3, 320, 320]},
    )
    
    runner.save_har(PARSED_HAR_PATH)
    print(f"✅ 파싱 완료. 결과 저장: {PARSED_HAR_PATH}\n")

if __name__ == "__main__":
    main()
