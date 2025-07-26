# Run from a recent fedora for now...
FROM fedora:42

ARG USERNAME=vscode

## Setup a user with sudo support
USER root

# Create a non-root user if one doesn't exist, and give it sudo rights
RUN if ! id -u ${USERNAME} > /dev/null 2>&1; then \
        groupadd --gid 1000 ${USERNAME} && \
        useradd --uid 1000 --gid 1000 --create-home --shell /bin/bash ${USERNAME} && \
        echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME}-nopasswd; \
    fi

# Install build & tool dependencies per README.md
RUN sudo dnf install -y cfitsio-devel eigen3-devel cmake extra-cmake-modules.noarch libXISF-devel kf5-kconfig-devel kf5-kdbusaddons-devel kf5-kdoctools-devel kf5-kguiaddons-devel kf5-ki18n-devel kf5-kiconthemes-devel kf5-kinit-devel kf5-kio-devel kf5-kjobwidgets-devel kf5-knewstuff-devel kf5-kplotting-devel kf5-ktexteditor-devel kf5-kwidgetsaddons-devel kf5-kwindowsystem-devel kf5-kxmlgui-devel libindi-devel libindi-static qt5-qtdeclarative-devel qt5-qtmultimedia-devel qt5-qtdatavis3d-devel qt5-qtsvg-devel wcslib-devel xplanet zlib-devel qt5-qtwebsockets-devel kf5-kcrash-devel kf5-knotifications-devel gsl-devel libnova-devel libcurl-devel

# Default user for devcontainer
USER ${USERNAME}

