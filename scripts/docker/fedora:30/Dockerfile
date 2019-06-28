FROM fedora:30
RUN dnf -y clean all                  \
        && dnf -y update              \
        && dnf -y install             \
                redhat-rpm-config     \
                git                   \
                cmake                 \
                make                  \
                gcc                   \
                apr-devel             \
                httpd-devel           \
                ORBit2-devel          \
                doxygen
