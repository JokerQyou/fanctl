[Unit]
Description=Auto control fan according to SoC temperature
DefaultDependencies=no
Wants=local-fs.target
ConditionPathExists=@CMAKE_INSTALL_PREFIX@/bin/fanctl

[Service]
Type=idle
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/fanctl
KillSignal=SIGINT
User=pi
Group=pi

[Install]
WantedBy=multi-user.target
