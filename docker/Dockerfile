FROM espressif/idf

ARG USER_ID
ARG GROUP_ID

RUN addgroup --gid $GROUP_ID user
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user

RUN mkdir -p //.ccache/tmp && chown -R $USER_ID //.ccache/tmp
USER user