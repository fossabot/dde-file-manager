/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
*/

#include <gtest/gtest.h>

#include "imagepreviewplugin.h"


class TestImagePreviewPlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_imagePreviewPlugin = new ImagePreviewPlugin();
    }

    void TearDown() override
    {
        delete m_imagePreviewPlugin;
        m_imagePreviewPlugin = nullptr;
    }

public:
    ImagePreviewPlugin * m_imagePreviewPlugin;
};

TEST_F(TestImagePreviewPlugin, use_create)
{
    auto *ptr = m_imagePreviewPlugin->create("dde-file-manager");
    EXPECT_TRUE(ptr != nullptr);
    if (ptr) {
        delete ptr;
        ptr = nullptr;
    }
}
