FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg:latest

# Увеличиваем таймаут pip и используем зеркало для надёжности
ENV PIP_DEFAULT_TIMEOUT=120
ENV PIP_INDEX_URL=https://pypi.tuna.tsinghua.edu.cn/simple

RUN apt-get update && apt-get install -y libmongoc-dev libbson-dev && rm -rf /var/lib/apt/lists/*

WORKDIR /service_template

COPY . .

RUN cmake -B build -S . && cmake --build build

EXPOSE 8080

CMD ["./build/service_template", "--config", "/service_template/configs/static_config.yaml"]
