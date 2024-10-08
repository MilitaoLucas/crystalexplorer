#pragma once
#include <QObject>
#include <ankerl/unordered_dense.h>
#include <QColor>
#include <QVector3D>

namespace cx::graphics {

enum class SelectionType {
    None,
    Atom,
    Bond,
    Surface,
};

struct SelectionResult {
    SelectionType type{SelectionType::None};
    quint32 identifier{0};
    int index{-1};
    quint32 secondaryIndex{0};
};

class RenderSelection: public QObject {
    Q_OBJECT
public:
    using IdentifierIndexMap = ankerl::unordered_dense::map<quint32, int>;
    using IndexIdentifierMap = ankerl::unordered_dense::map<int, quint32>;

    RenderSelection(QObject *parent = nullptr);

    [[nodiscard]] SelectionResult getSelectionFromColor(const QColor &) const;

    void clear(SelectionType);
    void clear();

    quint32 add(SelectionType type, int index);
    [[nodiscard]] QVector3D getColorFromId(quint32) const;

private:
    IdentifierIndexMap m_sphereToAtomIndex;
    IndexIdentifierMap m_atomToSphereIndex;

    IdentifierIndexMap m_cylinderToBondIndex;
    IndexIdentifierMap m_bondToCylinderIndex;

    IdentifierIndexMap m_meshToSurfaceIndex;
    IndexIdentifierMap m_surfaceToMeshIndex;
};

}
