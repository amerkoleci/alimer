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
    public void TestPrimitiveTypeMetadata()
    {
        bool found = MetadataRegistry.TryGetMetadata<int, IPrimitiveTypeMetadata>(out IPrimitiveTypeMetadata? metadata);
        Assert.That(found, Is.True);

        Assert.That(metadata, Is.Not.Null);
        Assert.That(metadata.Type, Is.Not.Null);
        //Assert.That(primitiveTypeMetadata.TypeInfo, Is.EqualTo(type));
    }

    [Test]
    public void TestEnumMetadata()
    {
        EnumTypeMetadata<TestEnum, int> metadata = new()
        {
            Items = __CreateItems_TestEnum()
        };
        MetadataRegistry.Register(metadata);

        IPrimitiveTypeMetadata intType = MetadataRegistry.GetPrimitiveTypeMetadata<int>();
        IEnumTypeMetadata storedMetadata = MetadataRegistry.GetMetadata<TestEnum, IEnumTypeMetadata>();
        Assert.That(storedMetadata, Is.Not.Null);
        Assert.That(storedMetadata.UnderlyingType, Is.EqualTo(intType));

        TestEnum enumCreated = (TestEnum)storedMetadata.CreateObject();
        Assert.That(enumCreated, Is.EqualTo(TestEnum.Field1));
    }

    private static IReadOnlyList<EnumItem> __CreateItems_TestEnum()
    {
        return new List<EnumItem>
        {
            new EnumItem(nameof(TestEnum.Field1), 0),
            new EnumItem(nameof(TestEnum.Field2), 1),
            new EnumItem(nameof(TestEnum.Field3), 2)
        };
    }


    public enum TestEnum
    {
        Field1,
        Field2,
        Field3
    }
}
