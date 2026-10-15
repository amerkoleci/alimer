// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace Alimer.Metadata;

[TestFixture(TestOf = typeof(MetadataRegistry))]
public class MetadataTests
{
    [Test]
    public void TestEnumMetadata()
    {
        MetadataRegistry.RegisterEnumType<TestEnum, int>();

        IEnumTypeMetadata storedMetadata = MetadataRegistry.GetMetadata<TestEnum, IEnumTypeMetadata>();
        Assert.That(storedMetadata, Is.Not.Null);
        Assert.That(storedMetadata.UnderlyingType, Is.EqualTo(typeof(int)));

        TestEnum enumCreated = (TestEnum)storedMetadata.CreateObject();
        Assert.That(enumCreated, Is.EqualTo(TestEnum.Field1));
    }

    public enum TestEnum
    {
        Field1,
        Field2,
        Field3
    }
}
