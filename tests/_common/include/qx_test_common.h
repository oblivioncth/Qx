// Macros
#define C_STR(q_str) q_str.toStdString().c_str()

// Test debug helpers (enables more helpful print-outs from test failures)
namespace Qx
{


/* Copy pasted from STARpp as example for this
    QString qnn(const QString& s) { return s.isNull() ? s : '"' + s + '"'; }

    QString elecResStr(const QList<Seat>& seats)
    {
        static const QString seatTempl = QStringLiteral(
            "\n"
            "Seat (%1)\n"
            "---------\n"
            "Winner: %2\n"
            "First Seed: %3\n"
            "Second Seed: %4\n"
            "Simultaneous: %5\n"
            "Overflow: {%6}\n"
            "\n"
        );

        QString resStr;
        resStr.reserve(seatTempl.size() * seats.size()); // Roughly final size

        for(auto i = 0; i < seats.size(); i++)
        {
            const Seat& seat = seats.at(i);
            QString winner = seat.winner();
            QualifierResult qr = seat.qualifierResult();

            resStr.append(
                seatTempl.arg(i)
                         .arg(qnn(seat.winner()), qnn(qr.firstSeed()), qnn(qr.secondSeed()))
                         .arg(qr.isSeededSimultaneously() ? "true" : "false")
                         .arg(qr.overflow().isEmpty() ? "" : '"' + Qx::String::join(qr.overflow(), R"(", ")") + '"')
            );
        }

        return resStr;
    }

    char* toString(const ExpectedElectionResult& eer)
    {
        QString string = elecResStr(eer.seats());
        return qstrdup(string.toUtf8().constData());
    }

    char* toString(const ElectionResult& er)
    {
        QString string = elecResStr(er.seats());
        return qstrdup(string.toUtf8().constData());
    }
*/
}
