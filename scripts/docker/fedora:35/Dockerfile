FROM fedora:35

RUN dnf -y install               \
           redhat-rpm-config     \
           git                   \
           cmake                 \
           make                  \
           gcc                   \
           apr-devel             \
           httpd-devel           \
           ORBit2-devel          \
           doxygen               \
    && dnf clean all             \
    && rm -rf /var/cache/yum
