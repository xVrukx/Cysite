# include <QStandardPaths>
#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>

class CyberNotepad : public QWidget {
    Q_OBJECT

public:
    CyberNotepad(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Cysite");
        resize(850, 550);
        
        // Ensure data directory exists
        notesDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (!QDir(notesDir).exists()) {
            QDir().mkpath(notesDir);
        }

        // Global Stylesheet (Fluorescent Green, Black, White, Rounded Corners)
        this->setStyleSheet(
            "QWidget { background-color: #051a05; color: #39ff14; font-family: 'Monospace', 'Segoe UI'; font-size: 13px; }"
            "QListWidget { background-color: #1a5c11; border: 2px solid #39ff14; border-radius: 10px; color: #ffffff; padding: 5px; }"
            "QListWidget::item:selected { background-color: #39ff14; color: #051a05; font-weight: bold; border-radius: 4px; }"
            "QTextEdit { background-color: #1a5c11; border: 2px solid #39ff14; border-radius: 10px; color: #ffffff; padding: 10px; font-size: 14px; }"
            "QLineEdit { background-color: #1a5c11; border: 2px solid #39ff14; border-radius: 6px; color: #ffffff; padding: 5px; }"
            "QPushButton { background-color: #051a05; border: 2px solid #39ff14; border-radius: 6px; color: #39ff14; padding: 6px; font-weight: bold; }"
            "QPushButton:hover { background-color: #39ff14; color: #051a05; }"
            "QPushButton:pressed { background-color: #1a5c11; color: #ffffff; }"
        );

        // Layouts
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        QVBoxLayout *sidebarLayout = new QVBoxLayout();
        QVBoxLayout *editorLayout = new QVBoxLayout();

        // Sidebar Elements (Left side)
        noteList = new QListWidget(this);
        QPushButton *addButton = new QPushButton("ADD NOTE", this);
        QPushButton *deleteButton = new QPushButton("DELETE NOTE", this);
        
        sidebarLayout->addWidget(noteList);
        sidebarLayout->addWidget(addButton);
        sidebarLayout->addWidget(deleteButton);

        // Editor Elements (Right side)
        titleInput = new QLineEdit(this);
        titleInput->setPlaceholderText("Note Title...");
        titleInput->setReadOnly(true); // Handled by CRUD list selection

        noteEditor = new QTextEdit(this);
        QPushButton *saveButton = new QPushButton("UPDATE / SAVE CHANGES", this);

        editorLayout->addWidget(titleInput);
        editorLayout->addWidget(noteEditor);
        editorLayout->addWidget(saveButton);

        // Assemble Layout
        mainLayout->addLayout(sidebarLayout, 1);
        mainLayout->addLayout(editorLayout, 2);

        // Connect Signals to Slots
        connect(addButton, &QPushButton::clicked, this, &CyberNotepad::addNote);
        connect(deleteButton, &QPushButton::clicked, this, &CyberNotepad::deleteNote);
        connect(saveButton, &QPushButton::clicked, this, &CyberNotepad::updateNote);
        connect(noteList, &QListWidget::itemClicked, this, &CyberNotepad::openNote);

        refreshNoteList();
    }

private slots:
    void refreshNoteList() {
        noteList->clear();
        QDir dir(notesDir);
        QStringList filters;
        filters << "*.txt";
        QStringList files = dir.entryList(filters, QDir::Files);
        for (const QString &file : files) {
            noteList->addItem(file.chopped(4)); // Remove .txt extension for UI display
        }
        titleInput->clear();
        noteEditor->clear();
        currentNoteName = "";
    }

    void addNote() {
        bool ok;
        QString name = QInputDialog::getText(this, "New Note", "Enter note title:", QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty()) return;

        QString filename = notesDir + "/" + name.trimmed() + ".txt";
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ""; // Create empty file
            file.close();
            refreshNoteList();
            
            // Auto-select the newly made note
            QList<QListWidgetItem *> items = noteList->findItems(name.trimmed(), Qt::MatchExactly);
            if (!items.isEmpty()) {
                noteList->setCurrentItem(items.first());
                openNote(items.first());
            }
        }
    }

    void openNote(QListWidgetItem *item) {
        if (!item) return;
        currentNoteName = item->text();
        titleInput->setText(currentNoteName);

        QString filename = notesDir + "/" + currentNoteName + ".txt";
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            noteEditor->setPlainText(in.readAll());
            file.close();
        }
    }

    void updateNote() {
        if (currentNoteName.isEmpty()) {
            QMessageBox::warning(this, "System Warning", "Please select or add a note first.");
            return;
        }

        QString filename = notesDir + "/" + currentNoteName + ".txt";
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << noteEditor->toPlainText();
            file.close();
            QMessageBox::information(this, "Saved", "Note updated successfully.");
        }
    }

    void deleteNote() {
        if (currentNoteName.isEmpty()) {
            QMessageBox::warning(this, "System Warning", "Please select a note to delete.");
            return;
        }

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm Delete", "Permanently delete this note?", QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QString filename = notesDir + "/" + currentNoteName + ".txt";
            QFile::remove(filename);
            refreshNoteList();
        }
    }

private:
    QListWidget *noteList;
    QLineEdit *titleInput;
    QTextEdit *noteEditor;
    QString notesDir;
    QString currentNoteName;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CyberNotepad window;
    window.show();
    return app.exec();
}

#include "main.moc"
