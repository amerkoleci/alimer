// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace Alimer.SourceGenerators;

public static class RoslynExtensions
{
    public static string ToDisplayString(this Accessibility accessibility)
        => accessibility switch
        {
            Accessibility.Public => "public",
            Accessibility.Private => "private",
            Accessibility.Protected => "protected",
            Accessibility.ProtectedAndInternal => "private internal",
            Accessibility.Internal => "internal",
            Accessibility.ProtectedOrInternal => "protected internal",
            _ => "public"
        };

    public static bool HasAttribute(this MemberDeclarationSyntax symbol, string name)
    {
        return symbol.GetAttribute(name) != null;
    }

    public static AttributeSyntax GetAttribute(this MemberDeclarationSyntax symbol, string name)
    {
        return symbol.AttributeLists.SelectMany(x => x.Attributes)
            .FirstOrDefault(y => y.Name.ToString() == name || y.Name.ToString() == name + "Attribute");
    }

    public static bool HasAttribute(this IMethodSymbol symbol, string name)
    {
        return symbol.GetAttribute(name) != null;
    }

    public static AttributeData? GetAttribute(this IMethodSymbol symbol, string name)
    {
        return symbol.GetAttributes().FirstOrDefault(y => y.AttributeClass!.Name == name || y.AttributeClass.Name == name + "Attribute" || y.AttributeClass.FullName() == name);
    }

    public static AttributeData? GetAttribute(this INamedTypeSymbol symbol, string name)
    {
        return symbol.GetAttributes().FirstOrDefault(y => y.AttributeClass!.Name == name || y.AttributeClass.Name == name + "Attribute" || y.AttributeClass.FullName() == name);
    }

    public static bool HasAttribute(this IPropertySymbol symbol, string name)
    {
        return symbol.GetAttribute(name) != null;
    }

    public static AttributeData? GetAttribute(this ISymbol symbol, string name)
    {
        return symbol.GetAttributes().FirstOrDefault(y => y.AttributeClass!.Name == name || y.AttributeClass.Name == name + "Attribute" || y.AttributeClass.FullName() == name);
    }

    public static AttributeData? GetAttribute(this IPropertySymbol symbol, string parentName, string name)
    {
        return symbol.GetAttributes().FirstOrDefault(y => y.AttributeClass!.ContainingType != null && y.AttributeClass.ContainingType.Name == parentName && (y.AttributeClass.Name == name || y.AttributeClass.Name == name + "Attribute"));
    }

    public static bool DerivesFrom(this ITypeSymbol symbol, string name, bool exact = false)
    {
        if (symbol == null) return false;
        if (exact && symbol.FullName() == name) return true;
        if (!exact && symbol.FullName().StartsWith(name)) return true;

        foreach (var i in symbol.AllInterfaces)
        {
            if (exact && i.FullName() == name) return true;
            if (!exact && i.FullName().StartsWith(name)) return true;
        }

        return symbol.BaseType!.DerivesFrom(name, exact);
    }

    public static bool DerivesFrom(this ITypeSymbol symbol, ITypeSymbol search)
    {
        if (symbol == null) return false;
        if (symbol.MetadataName == search.MetadataName)
            return true;
        return symbol.BaseType.DerivesFrom(search);
    }

    public static bool IsAutoProperty(this IPropertySymbol propertySymbol)
    {
        var fields = propertySymbol.ContainingType.GetMembers().OfType<IFieldSymbol>();
        return fields.Any(field => SymbolEqualityComparer.Default.Equals(field.AssociatedSymbol, propertySymbol));
    }
}
