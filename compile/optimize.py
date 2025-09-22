# General imports used throughout the tutorial
# file operations
import json
import os
import random

import numpy as np
import tensorflow as tf
from IPython.display import SVG
from matplotlib import patches
from matplotlib import pyplot as plt
from PIL import Image
from tensorflow.python.eager.context import eager_mode

# import the hailo sdk client relevant classes
from hailo_sdk_client import ClientRunner, InferenceContext

#%matplotlib inline

IMAGES_TO_VISUALIZE = 5

# First, we will prepare the calibration set. Resize the images to the correct size and crop them.
def preproc(image, output_height=320, output_width=320, resize_side=320):
    """imagenet-standard: aspect-preserving resize to 256px smaller-side, then central-crop to 224px"""
    with eager_mode():
        h, w = image.shape[0], image.shape[1]
        scale = tf.cond(tf.less(h, w), lambda: resize_side / h, lambda: resize_side / w)
        resized_image = tf.compat.v1.image.resize_bilinear(tf.expand_dims(image, 0), [int(h * scale), int(w * scale)])
        cropped_image = tf.compat.v1.image.resize_with_crop_or_pad(resized_image, output_height, output_width)

        return tf.squeeze(cropped_image)

"""
images_path = "calib_set"
images_list = [img_name for img_name in os.listdir(images_path) if os.path.splitext(img_name)[1] == ".jpg"]

num_to_sample = 1024

# 만약 전체 이미지 수가 1024개보다 적으면, 가능한 모든 이미지를 사용
if len(images_list) < num_to_sample:
    print(f"경고: 이미지 개수({len(images_list)}개)가 요청한 샘플 개수({num_to_sample}개)보다 적습니다. 모든 이미지를 사용합니다.")
    num_to_sample = len(images_list)

# 전체 이미지 목록에서 지정된 개수만큼 랜덤으로 샘플링합니다.
images_list = random.sample(images_list, num_to_sample)

print(f"전체 이미지 중 {len(images_list)}개를 랜덤으로 선택했습니다.")

calib_dataset = np.zeros((len(images_list), 320, 320, 3))
for idx, img_name in enumerate(sorted(images_list)):
    img = np.array(Image.open(os.path.join(images_path, img_name)))
    img_preproc = preproc(img)
    calib_dataset[idx, :, :, :] = img_preproc.numpy()

np.save("calib_set.npy", calib_dataset)
"""
# Second, we will load our parsed HAR from the Parsing Tutorial

model_name = "best"
hailo_model_har_name = f"{model_name}_parsed.har"
assert os.path.isfile(hailo_model_har_name), "Please provide valid path for HAR file"
runner = ClientRunner(har=hailo_model_har_name)
# By default it uses the hw_arch that is saved on the HAR. For overriding, use the hw_arch flag.

# Now we will create a model script, that tells the compiler to add a normalization on the beginning
# of the model (that is why we didn't normalize the calibration set;
# Otherwise we would have to normalize it before using it)

# Batch size is 8 by default
#alls = "normalization1 = normalization([0.0, 0.0, 0.0], [255.0, 255.0, 255.0])\n"

# Load the model script to ClientRunner so it will be considered on optimization
#runner.load_model_script(alls)

# 2. 모델 스크립트 적용 (NMS, 정규화 등 추가)
ALLS_SCRIPT_PATH = 'best.alls'
runner.load_model_script(ALLS_SCRIPT_PATH)
print(f"모델 스크립트 적용 완료: {ALLS_SCRIPT_PATH}")

# 3. 캘리브레이션 데이터로 실제 양자화 수행
CALIBRATION_DATA_PATH = 'calib_set.npy'
print(f"캘리브레이션 데이터 로딩: {CALIBRATION_DATA_PATH}")
calibration_data = np.load(CALIBRATION_DATA_PATH)
    
# Call Optimize to perform the optimization process
runner.optimize(calibration_data)

# Save the result state to a Quantized HAR file
quantized_model_har_path = f"{model_name}_quantized_model.har"
if(runner.save_har(quantized_model_har_path)):
    print("Success!!")
