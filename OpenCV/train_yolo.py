# train_yolo.py

from ultralytics import YOLO

def main():
    # 1. 사전 학습된 YOLOv8s 모델 로드
    # 'yolov8s.pt'는 사물 탐지에 대한 기본적인 학습이 되어있는 모델입니다.
    # 처음부터 학습하는 것보다 훨씬 빠르고 성능이 좋습니다. (전이 학습, Transfer Learning)
    print("사전 학습된 YOLOv8s 모델을 로드합니다...")
    model = YOLO('yolov8s.pt')

    # 2. 커스텀 데이터로 모델 학습 시작
    # data: data.yaml 파일 경로
    # epochs: 전체 데이터셋을 몇 번 반복 학습할지 결정 (테스트용으로 낮게 시작, 실제로는 100 이상)
    # imgsz: 학습에 사용할 이미지 크기. Hailo에 배포할 크기와 맞추는 것이 좋습니다.
    print("커스텀 데이터로 모델 학습을 시작합니다...")
    model.train(data='data.yaml', 
                epochs=300,
                imgsz=320,      # 320->640
                batch=1,      # 64->128
                patience=50,

                # --- 옵티마이저 및 학습률 스케줄러 설정 ---
                optimizer='AdamW',  # 옵티마이저 종류 선택 (Adam, AdamW, SGD 등)
                lr0=0.001,           # 초기 학습률 (Initial Learning Rate), 0.001->0.01
                lrf=0.01,           # 최종 학습률 (Final Learning Rate = lr0 * lrf)
                #momentum=0.937,     # SGD momentum/Adam beta1
                #weight_decay=0.0005,# 가중치 감쇠 (과적합 방지)

                # --- 데이터 증강 (Data Augmentation) 설정 ---
                degrees=2.0,       # 이미지 회전 각도 (±10도)  10->2
                translate=0.1,      # 이미지 이동 비율 (±10%)
                scale=0.1,          # 이미지 확대/축소 비율 (±50%) 0.5->0.1
                shear=0,          # 이미지 전단 변환 각도 (±5도) 5->0
                perspective=0.00,  # 이미지 원근 변환 0.001->0
                flipud=0,         # 상하 반전 확률 0.5->0
                fliplr=0.5,         # 좌우 반전 확률 (기본값)
                mosaic=1.0,         # 모자이크 증강 사용 확률 (학습 초기에 효과적)
                mixup=0,          # MixUp 증강 사용 확률 0.1->0

                # --- 손실 함수 (Loss Function) 가중치 설정 ---
                #box=7.5,            # 바운딩 박스 손실(box_loss)에 대한 가중치
                #cls=0.5,            # 분류 손실(cls_loss)에 대한 가중치
                #dfl=1.5,             # DFL 손실(dfl_loss)에 대한 가중치
                name='drone_detector_yolov8s_dataset3') # 결과가 저장될 폴더 이름

    print(f"\n학습완료")

if __name__ == '__main__':
    main()
