// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace Alimer.Metadata;

[TestFixture(TestOf = typeof(MetadataRegistry))]
public class ObjectMetadataTests
{
    [Test]
    public void TestObjectMetadata()
    {
        ObjectTypeMetadata metadata = new();
        MetadataRegistry.Register(metadata);

        IPrimitiveTypeMetadata intType = MetadataRegistry.GetPrimitiveTypeMetadata<int>();
        IEnumTypeMetadata storedMetadata = MetadataRegistry.GetMetadata<TestEnum, IEnumTypeMetadata>();
        Assert.That(storedMetadata, Is.Not.Null);
        Assert.That(storedMetadata.UnderlyingType, Is.EqualTo(intType));

        TestEnum enumCreated = (TestEnum)storedMetadata.CreateObject();
        Assert.That(enumCreated, Is.EqualTo(TestEnum.Field1));
    }
}
