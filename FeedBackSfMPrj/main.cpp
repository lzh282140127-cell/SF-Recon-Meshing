// SF-Recon Meshing
// Copyright (C) 2026 SF-Recon Authors
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version. See the LICENSE file in the repository root.

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    InitializeGlog(argv);

    QApplication app(argc, argv);

    MainWindow w;
    w.show();
    return app.exec();

}
