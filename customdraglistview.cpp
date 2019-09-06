#include "customdraglistview.h"

CustomDragListView::CustomDragListView(QWidget* parent)
    :
      QListView(parent)
{

}

void CustomDragListView::setCurrentlyLoadedDocument(PdfUtil *document)
{
    currentlyLoadedDocument = document;
}

void CustomDragListView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.count() > 0) {
        QMimeData *data = model()->mimeData(indexes);
        if (!data)
            return;

        if(indexes.length()>1)
        {
            QAbstractItemView::startDrag(supportedActions);
        } else {
            if(currentlyLoadedDocument != nullptr)
            {
                QRect rect;
                QVariant m_data = model()->data(indexes.first(), Qt::EditRole);
                int pageNum = 0;
                if(strcmp(m_data.typeName(), "PdfPageRangeSpecificator*") == 0)
                {
                    PdfPageRangeSpecificator* intervalData = qvariant_cast<PdfPageRangeSpecificator*>(m_data);
                    if(intervalData->getAllPages().length()>1)
                    {
                        QAbstractItemView::startDrag(supportedActions);
                        return;
                    } else pageNum = intervalData->getAllPages().first();
                } else pageNum = qvariant_cast<QString>(m_data).toInt() - 1;

                PdfRenderedPage* pdfPage = currentlyLoadedDocument->GetPdfRenderedPage(pageNum);
                QImage* image = pdfPage->getImage();
                QPixmap pixmap = QPixmap::fromImage(*image).scaled(210, 297, Qt::KeepAspectRatio);

                QPixmap output(pixmap.size());
                output.fill(Qt::transparent);
                QPainter painter(&output);
                painter.setOpacity(0.7);
                painter.drawPixmap(0, 0, pixmap);
                painter.setPen(QColor(0, 0, 0, 127));
                int right = pixmap.width() - 1;
                int bottom = pixmap.height() - 1;
                painter.drawLine(0, 0, right, 0); // Top border
                painter.drawLine(right, 0, right, bottom); // Right border
                painter.drawLine(right, bottom, 0, bottom); // Bottom border
                painter.drawLine(0, bottom, 0, 0); // Left border
                painter.end();

                rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);
                QDrag *drag = new QDrag(this);
                drag->setPixmap(output);
                drag->setMimeData(data);
                drag->setHotSpot(QPoint(-48,-48)); //d->pressedPosition - rect.topLeft());
                Qt::DropAction defAction = Qt::IgnoreAction;
                if (defaultDropAction() != Qt::IgnoreAction && (supportedActions & defaultDropAction()))
                    defAction = defaultDropAction();
                else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
                    defAction = Qt::CopyAction;
                drag->exec(supportedActions, defAction);

                delete image;
                delete pdfPage;
            } else QAbstractItemView::startDrag(supportedActions);
        }
    }
}