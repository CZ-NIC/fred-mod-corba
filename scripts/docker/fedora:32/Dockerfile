FROM fedora:32

RUN dnf -y clean all                  \
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
