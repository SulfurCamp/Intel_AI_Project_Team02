# 인텔 AI 프로젝트 2조 – 드론 탐지 터렛

## 프로젝트 배경과 목표
엣지 디바이스에서 빠르고 안정적으로 작동하는 드론 감지·추적 시스템을 구축  
침입 징후를 조기에 식별하고 대응할 수 있는 프로토타입을 만드는 것을 목표

## 팀 소개
- 황진영(팀장) – PM, GUI 설계
- 김국현 – Hailo 컴파일 파이프라인, AI 최적화/성능 분석
- 정태윤 – 팬·틸트 제어, GUI의 Yocto 포팅 및 통신 연동
- 윤동준 – 영상 처리, 카메라 스트리밍, 경로 계산 로직
- 박성준 – Hailo 애플리케이션, 후처리 및 추론 서비스 통합

## 시스템 아키텍처 개요
1. **AI 추론**: CUDA(GPU) 혹은 Hailo-8 NPU에서 YOLOv8s 모델로 드론을 실시간 감지합니다.
2. **데이터 라우팅**: RealSense D435가 제공하는 컬러·깊이·IMU 값을 융합해 영상 처리 및 위험도 판단을 수행합니다.
3. **경로 결정**: 탐지된 드론의 중심 좌표를 9방향으로 분류해 이동 경로를 계산하고 UART로 STM32 팬·틸트를 제어합니다.
4. **시각화**: 동일 데이터를 Socket으로 전송해 Qt GUI에 x/y/z 보정 좌표와 알림 상태를 표시하며, MJPG 스트리밍 서버로 카메라 영상을 공유합니다.

시퀀스 다이어그램과 아키텍처 다이어그램은 발표 자료에 포함되어 있으며, 각 모듈이 이벤트 흐름에 따라 유기적으로 동작하도록 설계했습니다.
시퀸스 다이어그램
![alt text](<스크린샷 2025-09-27 19-42-35.png>)
## 구현 상세
### AI 추론 및 모델 구성
- 엣지 환경에 유리한 **YOLOv8s**를 선택하고 라벨을 `drone` 단일 클래스로 통합해 혼동을 줄였습니다.
- 이미지 크기를 640→320으로 축소해 지연을 최소화했으며, `OpenCV/train_yolo.py`에서 AdamW, Mosaic 등을 활용한 전이 학습 파이프라인을 구축했습니다.

### Hailo 워크플로우
- `compile/parse.py → optimize.py → compile.py` 순으로 ONNX → HAR → HEF를 생성하고, `best.alls`와 `hailo_config.yaml`로 on-device NMS를 활성화했습니다.
- 학습된 HEF를 `AI_Application/object_detection_fin.py`에서 호출 가능한 `example()` API로 감싸 다른 모듈이 쉽게 사용할 수 있도록 했습니다.
- Hailo-8은 NVIDIA GPU와 유사한 추론 성능을 제공하면서 전력 효율이 높아, Raspberry Pi 기반 엣지 배포에 유리한 결과를 확인했습니다.

### CUDA 비교 환경
- `OpenCV/app_cuda.py`는 동일한 YOLOv8 모델을 CUDA에서 실행해 Hailo와의 성능·전력 지표를 비교합니다.
- 풍부한 라이브러리와 병렬 처리 장점을 활용해 초기 개발과 디버깅에 이용했으며, 결과 데이터 포맷은 Hailo 버전과 동일합니다.

### 데이터 변환 & 영상 처리
- `OpenCV/app_hailo.py`에서 RealSense 컬러/깊이 스트림을 30fps로 수집하고, Flask 기반 MJPG 스트림으로 외부에 제공했습니다.
- 드론 중심 좌표를 3×3 Keymap(QWE/ASD/ZXC)으로 변환해 위험 방위를 정의하고, 거리 기반 위험도 레벨을 추가했습니다.
- Socket 프로토콜(`[QTCLIENT]{sector}@{depthFlag}@{label}`)을 정의해 GUI, 서버, STM32가 동일한 포맷으로 데이터를 교환합니다.

### 팬·틸트 제어 (STM32)
- `STM32/pan_tilt/Core/Src/main.c`는 UART 인터럽트로 명령을 수신하고, 듀얼 PWM 채널을 이용해 팬·틸트 서보를 제어합니다.
- 명령은 9방향 문자와 거리 레벨(E@3 등)로 구성되며, 거리 수준에 따라 30°/20°/10° 단계로 각도를 조절합니다.

### GUI 및 Yocto 배포
- Qt `MainWidget`은 메뉴, DB/서버 모니터링, 3D 그래프 탭을 제공하며 배경 영상·음악으로 UX를 강화했습니다.
- `sockclient.cpp`가 실시간 좌표를 수신하고 `tab3dgraph`에서 QtDataVisualization으로 드론 궤적을 표현합니다.
- Yocto 기반 Custom Linux 이미지에 GUI를 포팅해 임베디드 환경에서도 일관된 UI를 제공했습니다.

## 프로젝트 결과
- Hailo-8 기반 드론 감지 파이프라인 구축 및 CUDA 대비 전력 효율성 입증
- 터렛 이동 경로 전달 알고리즘과 RealSense 기반 보정 필터 완성
- STM32 팬·틸트 서보 제어 및 UART 프로토콜 확립
- Qt UI/SQLite/스트리밍 서버로 통합 모니터링 환경 구현
- 위험 드론 알림 시나리오를 실시간 스트리밍과 데이터 로그로 검증

## 시행착오와 해결
| 이슈 | 해결 방안 |
| --- | --- |
| Hailo 컴파일에서 NMS 삽입 시 CLI 오류 | Hailo 지원과 협의 후 Python 기반 파이프라인으로 전환, `best.alls`/`hailo_config.yaml`로 on-chip NMS 구성 |
| Application ↔ HEF 설정 불일치 | 후처리 스크립트와 `config.json`을 재정비해 Output shape를 일치시키고, `example()` API로 통합 |
| 3D 좌표 기준 드리프트 | RealSense IMU를 활용한 보정 필터 적용으로 GUI 3D 그래프의 기준을 고정 |
| Raspberry Pi에서 Hailo 추론 시 통신·스트리밍 지연 | 병목 분석 후 프로세스 우선순위 조정, 프레임 버퍼 최적화, 경량화된 메시 포맷 도입 |

## 개발 프로세스와 협업
- Jira 기반 애자일 방식으로 Epic → Task를 분해하고 담당자를 지정해 진행 상황을 공유했습니다.
- 주간 스탠드업과 리뷰 미팅을 통해 이슈를 조기에 발견했으며, Git 기반 브랜치 전략으로 모듈 간 충돌을 최소화했습니다.
- 발표 자료에는 시퀀스 다이어그램과 각 Task의 WBS가 포함되어 프로젝트 흐름을 시각적으로 제시했습니다.

## 향후 계획
- Raspberry Pi보다 고성능 엣지 보드로 교체해 추론·통신 처리 속도를 향상
- 드론 이외의 헬기, 항공기 등 다중 클래스 라벨 확장 및 다양한 모델(SSD 등) 비교 평가
- 위험도 판단을 강화하기 위한 추가 센서(레이더/음향) 융합과 지능형 경보 로직 개발

## 실행 가이드
1. **Hailo 환경 준비**: `AI_Application/README.md` 를 참고하여 HailoRT, 드라이버, Python wheel 설치 후 venv 구성
2. **모델 컴파일**
   ```bash
   cd compile
   python3 parse.py
   python3 optimize.py
   python3 compile.py
   ```
3. **엣지 추론 서비스**
   ```bash
   # Hailo
   source ~/hailo-8/hailo-8/.venv/bin/activate
   export HAILO_HEF=/path/to/best.hef
   export HAILO_LABELS=/path/to/drone.txt
   python3 OpenCV/app_hailo.py

   # CUDA 비교
   python3 OpenCV/app_cuda.py --model best.pt
   ```
4. **백엔드/GUI**
   - `Main_PC` 폴더에서 `make && ./iot_server`
   - Qt Creator 혹은 `cmake --build GUI/build` 후 실행
5. **STM32 펌웨어**: CubeIDE에서 `STM32/pan_tilt/pan_tilt.ioc`를 열어 빌드·플래시하고, `pan_tilt_uart_test` 로 통신을 검증합니다.

## 검증 및 모니터링
- Flask MJPG 스트림과 Qt 3D 그래프로 드론 추적 상태를 실시간 확인합니다.
- UART/Socket 패킷 로그를 모니터링하여 팬·틸트 제어 및 GUI 업데이트가 정상인지 점검합니다.
- Jira/Git 로그로 Task 진행, 회고, 향후 개선점을 관리했습니다.

## 참고 자료
- 최종 발표: [`Intel_AI_2조_SCV 최종.pdf`](Intel_AI_2조_SCV%20최종.pdf)
- Hailo Application Code Examples & Model Zoo 참고 링크는 `AI_Application/README.md`에 정리되어 있습니다.
- STM32CubeIDE, Qt, Ultralytics YOLO 공식 문서

여러 시행착오를 겪으며 영상 처리, 이동 경로 보정, Hailo 최적화 역량을 높였고, 협업과 이슈 관리를 통해 통합 드론 대응 시스템을 완성했습니다.
