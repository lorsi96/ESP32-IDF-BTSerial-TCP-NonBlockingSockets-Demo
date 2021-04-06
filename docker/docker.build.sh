docker build -t lorsi/pdm \
  --build-arg USER_ID=$(id -u) \
  --build-arg GROUP_ID=$(id -g) .