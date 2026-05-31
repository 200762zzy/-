#include <QtTest>
#include "test_deepseekapi.h"
#include "test_docxhelper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int allStatus = 0;

    TestDeepSeekApi testApi;
    allStatus |= QTest::qExec(&testApi, argc, argv);

    TestDocxHelper testDocx;
    allStatus |= QTest::qExec(&testDocx, argc, argv);

    return allStatus;
}
