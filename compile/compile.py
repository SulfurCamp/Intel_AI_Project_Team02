from hailo_sdk_client import ClientRunner

model_name = "best"
quantized_model_har_path = f"{model_name}_quantized_model.har"

runner = ClientRunner(har=quantized_model_har_path)
# By default it uses the hw_arch that is saved on the HAR. It is not recommended to change the hw_arch after Optimization.

hef = runner.compile()

file_name = f"{model_name}.hef"
with open(file_name, "wb") as f:
    f.write(hef)

har_path = f"{model_name}_compiled_model.har"
runner.save_har(har_path)
#!hailo profiler {har_path}
