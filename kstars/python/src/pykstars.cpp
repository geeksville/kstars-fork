/*
    SPDX-FileCopyrightText: 2021 Valentin Boettcher <hiro at protagon.space; @hiro98:tchncs.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "skyobjects/skypoint.h"
#include "skymesh.h"
#include "htmesh/MeshIterator.h"
#include "cachingdms.h"
#include "sqlstatements.cpp"
#include "catalogobject.h"
#include "catalogsdb.h"
#include <iostream>

using namespace pybind11::literals;
namespace py = pybind11;
namespace pybind11
{
namespace detail
{
template <>
struct type_caster<QString>
{
  public:
    PYBIND11_TYPE_CASTER(QString, _("QString"));

    bool load(handle src, bool)
    {
        try
        {
            value = QString::fromStdString(src.cast<std::string>());
        }
        catch (const py::cast_error &)
        {
            return false;
        }

        return !PyErr_Occurred();
    }

    static handle cast(QString src, return_value_policy /* policy */, handle /* parent */)
    {
        const handle *obj = new py::object(py::cast(src.toUtf8().constData()));
        return *obj;
    }
};

template <>
struct type_caster<QDateTime>
{
  public:
    PYBIND11_TYPE_CASTER(QDateTime, _("QDateTime"));

    bool load(handle src, bool)
    {
        try
        {
            value = QDateTime::fromMSecsSinceEpoch(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    src.cast<std::chrono::system_clock::time_point>().time_since_epoch())
                    .count());
        }
        catch (const py::cast_error &)
        {
            return false;
        }

        return !PyErr_Occurred();
    }

    static handle cast(QDateTime src, return_value_policy /* policy */,
                       handle /* parent */)
    {
        const handle *obj = new py::object(py::cast(std::chrono::system_clock::time_point(
            std::chrono::milliseconds(src.currentMSecsSinceEpoch()))));
        return *obj;
    }
};
} // namespace detail
} // namespace pybind11

/**
 * @struct Indexer
 * Provides a simple wrapper to generate trixel ids from python code.
 */
struct Indexer
{
    Indexer(int level) : m_mesh{ SkyMesh::Create(level) } {};

    int getLevel() const { return m_mesh->level(); };
    void setLevel(int level) { m_mesh = SkyMesh::Create(level); };

    int getTrixel(double ra, double dec, bool convert_epoch = false) const
    {
        SkyPoint p{ dms(ra), dms(dec) };
        if (convert_epoch)
        {
            p.B1950ToJ2000();
            p = SkyPoint{ p.ra(), p.dec() }; // resetting ra0, dec0
        }

        return m_mesh->index(&p);
    };

    std::vector<int> getTrixels(double ra, double dec, double radius, bool convert_epoch = false) const
    {
        SkyPoint p{ dms(ra), dms(dec) };
        if (convert_epoch)
        {
            p.B1950ToJ2000();
            p = SkyPoint{ p.ra(), p.dec() }; // resetting ra0, dec0
        }

        m_mesh->index(&p, radius, DRAW_BUF);

        std::vector<int> trixels;

        MeshIterator region(m_mesh, DRAW_BUF);
        while (region.hasNext()) {
            trixels.push_back(region.next());
        }

        return trixels;
    }

    SkyMesh *m_mesh;
};

/**
 * @struct CoordinateConversion
 * Provides a wrapper to perform some coordinate conversion (notably, precession)
 */
struct CoordinateConversion
{
    static std::pair<double, double> precess(const double ra, const double dec, const double src_jyear, const double dest_jyear)
    {
        SkyPoint p{ dms(ra), dms(dec) };
        const auto src_jd = KStarsDateTime::epochToJd(src_jyear, KStarsDateTime::JULIAN);
        const auto dest_jd = KStarsDateTime::epochToJd(dest_jyear, KStarsDateTime::JULIAN);
        p.precessFromAnyEpoch(src_jd, dest_jd);
        return {p.ra().reduce().Degrees(), p.dec().reduce().Degrees()};
    }

    static std::pair<py::array_t<double>, py::array_t<double>> precess_v(const py::array_t<double> ra, const py::array_t<double> dec, const double src_jyear, const double dest_jyear)
    {
        if (ra.ndim() != 1 || dec.ndim() !=1) {
            throw std::runtime_error("Number of dimensions must be one");
        }

        if (ra.size() != dec.size()) {
            throw std::runtime_error("Sizes of ra and dec arrays are not equal!");
        }

        const auto N = ra.size();

        auto dest_ra = py::array_t<double>(N);
        auto dest_dec = py::array_t<double>(N);

        const auto src_jd = KStarsDateTime::epochToJd(src_jyear, KStarsDateTime::JULIAN);
        const auto dest_jd = KStarsDateTime::epochToJd(dest_jyear, KStarsDateTime::JULIAN);
        const KSNumbers src_num(src_jd);
        const KSNumbers dest_num(dest_jd);
        const auto matrix = dest_num.p2() * src_num.p1();

        py::buffer_info buf_ra_in = ra.request(),
            buf_dec_in = dec.request();

        py::buffer_info buf_ra_out = dest_ra.request(),
            buf_dec_out = dest_dec.request();

        const double* ra_in = static_cast<const double *>(buf_ra_in.ptr);
        const double* dec_in = static_cast<const double *>(buf_dec_in.ptr);
        double* ra_out = static_cast<double *>(buf_ra_out.ptr);
        double* dec_out = static_cast<double *>(buf_dec_out.ptr);

        // FIXME: Parallelize this
        for (std::size_t i = 0; i < N; ++i) {
            double cosRA, sinRA, cosDec, sinDec;
            dms(ra_in[i]).SinCos(sinRA, cosRA);
            dms(dec_in[i]).SinCos(sinDec, cosDec);
            Eigen::Vector3d v{cosRA * cosDec, sinRA * cosDec, sinDec}, s;
            s.noalias() = matrix * v;
            ra_out[i] = atan2(s[1], s[0]) / dms::DegToRad;
            dec_out[i] = asin(s[2]) / dms::DegToRad;
        };

        return {dest_ra, dest_dec};
    }
};

///////////////////////////////////////////////////////////////////////////////
//                                   PYBIND                                  //
///////////////////////////////////////////////////////////////////////////////

const CatalogObject DEFAULT_CATALOG_OBJECT{};

template <typename T>
T cast_default(const py::object &value, const T &default_value)
{
    try
    {
        return py::cast<T>(value);
    }
    catch (const py::cast_error &)
    {
        return default_value;
    }
}

PYBIND11_MODULE(pykstars, m)
{
    m.doc() = "Thin bindings for KStars to facilitate trixel indexation from python.";

    py::class_<Indexer>(m, "Indexer")
        .def(py::init<int>(), "level"_a,
             "Initializes an `Indexer` with the given `level`.\n"
             "If the level is greater then approx. 10 the initialization can take some "
             "time.")
        .def_property("level", &Indexer::getLevel, &Indexer::setLevel,
                      "Sets the level of the HTMesh/SkyMesh used to index points.")
        .def(
            "get_trixel", &Indexer::getTrixel, "ra"_a, "dec"_a, "convert_epoch"_a = false,
            "Calculates the trixel number from the right ascention and the declination.\n"
            "The epoch of coordinates is assumed to be J2000.\n\n"
            "If the epoch is B1950, `convert_epoch` has to be set to `True`.")
        .def("get_trixel", py::vectorize(&Indexer::getTrixel))
        .def(
            "get_trixels", &Indexer::getTrixels, "ra"_a, "dec"_a, "radius"_a, "convert_epoch"_a = false,
            "Returns the trixels spanned by a circular aperture of the given radius around the given ra and dec.\n"
            "The epoch of coordinates is assumed to be J2000.\n\n"
            "If the epoch is B1950, `convert_epoch` has to be set to `True`.")
        .def("__repr__", [](const Indexer &indexer) {
            std::ostringstream lvl;
            lvl << indexer.getLevel();
            return "<Indexer level=" + lvl.str() + ">";
        });

    py::class_<CoordinateConversion>(m, "CoordinateConversion")
        .def_static("precess", &CoordinateConversion::precess, "ra"_a, "dec"_a, "src_jyear"_a, "dest_jyear"_a,
                    "Converts the provided ra and dec (both decimal degrees) from the Julian-year equinox `src_jyear` to"
                    " the `dest_year` equinox, returning a tuple (ra, dec) in decimal degrees.")
        .def_static("precess", &CoordinateConversion::precess_v,
                    "ras"_a, "decs"_a, "src_jyear"_a, "dest_jyear"_a,
                    "Converts the provided ra and dec arrays (both decimal degrees) from the Julian-year equinox `src_jyear`"
                    " to the `dest_year` equinox, returning a tuple of arrays (ra, dec) in decimal degrees.");

    {
        using namespace CatalogsDB;
        py::class_<DBManager>(m, "DBManager")
            .def(py::init([](const std::string &filename) {
                     return new DBManager(QString::fromStdString(filename));
                 }),
                 "filename"_a)
            .def(
                "register_catalog",
                [](DBManager &self, const py::dict &cat) {
                    return self.register_catalog(
                        py::cast<int>(cat["id"]), py::cast<QString>(cat["name"]),
                        py::cast<bool>(cat["mut"]), py::cast<bool>(cat["enabled"]),
                        py::cast<double>(cat["precedence"]),
                        py::cast<QString>(cat["author"]),
                        py::cast<QString>(cat["source"]),
                        py::cast<QString>(cat["description"]),
                        py::cast<int>(cat["version"]), py::cast<QString>(cat["color"]),
                        py::cast<QString>(cat["license"]),
                        py::cast<QString>(cat["maintainer"]),
                        py::cast<QDateTime>(cat["timestamp"]));
                },
                "catalog"_a)
            .def(
                "update_catalog_meta",
                [](DBManager &self, const py::dict &cat) {
                    return self.update_catalog_meta(
                        { py::cast<int>(cat["id"]), py::cast<QString>(cat["name"]),
                          py::cast<double>(cat["precedence"]),
                          py::cast<QString>(cat["author"]),
                          py::cast<QString>(cat["source"]),
                          py::cast<QString>(cat["description"]),
                          py::cast<bool>(cat["mut"]), py::cast<bool>(cat["enabled"]),
                          py::cast<int>(cat["version"]), py::cast<QString>(cat["color"]),
                          py::cast<QString>(cat["license"]),
                          py::cast<QString>(cat["maintainer"]),
                          py::cast<QDateTime>(cat["timestamp"]) });
                },
                "catalog"_a)
            .def("__repr__",
                 [](const DBManager &manager) {
                     return QString("<DBManager filename=\"" + manager.db_file_name() +
                                    "\">");
                 })
            .def("update_catalog_views", &DBManager::update_catalog_views)
            .def("compile_master_catalog", &DBManager::compile_master_catalog)
            .def("dump_catalog", &DBManager::dump_catalog, "catalog_id"_a, "file_path"_a)
            .def("import_catalog", &DBManager::import_catalog, "file_path"_a,
                 "overwrite"_a)
            .def("remove_catalog", &DBManager::remove_catalog, "catalog_id"_a);

        py::register_exception<DatabaseError>(m, "DatabaseError");
    }

    py::enum_<SkyObject::TYPE>(m, "ObjectType", "The types of CatalogObjects",
                               py::arithmetic())
        .value("STAR", SkyObject::STAR)
        .value("CATALOG_STAR", SkyObject::CATALOG_STAR)
        .value("PLANET", SkyObject::TYPE::PLANET)
        .value("OPEN_CLUSTER", SkyObject::TYPE::OPEN_CLUSTER)
        .value("GLOBULAR_CLUSTER", SkyObject::TYPE::GLOBULAR_CLUSTER)
        .value("GASEOUS_NEBULA", SkyObject::TYPE::GASEOUS_NEBULA)
        .value("PLANETARY_NEBULA", SkyObject::TYPE::PLANETARY_NEBULA)
        .value("SUPERNOVA_REMNANT", SkyObject::TYPE::SUPERNOVA_REMNANT)
        .value("GALAXY", SkyObject::TYPE::GALAXY)
        .value("COMET", SkyObject::TYPE::COMET)
        .value("ASTEROID", SkyObject::TYPE::ASTEROID)
        .value("CONSTELLATION", SkyObject::TYPE::CONSTELLATION)
        .value("MOON", SkyObject::TYPE::MOON)
        .value("ASTERISM", SkyObject::TYPE::ASTERISM)
        .value("GALAXY_CLUSTER", SkyObject::TYPE::GALAXY_CLUSTER)
        .value("DARK_NEBULA", SkyObject::TYPE::DARK_NEBULA)
        .value("QUASAR", SkyObject::TYPE::QUASAR)
        .value("MULT_STAR", SkyObject::TYPE::MULT_STAR)
        .value("RADIO_SOURCE", SkyObject::TYPE::RADIO_SOURCE)
        .value("SATELLITE", SkyObject::TYPE::SATELLITE)
        .value("SUPERNOVA", SkyObject::TYPE::SUPERNOVA)
        .value("NUMBER_OF_KNOWN_TYPES", SkyObject::TYPE::NUMBER_OF_KNOWN_TYPES)
        .value("TYPE_UNKNOWN", SkyObject::TYPE::TYPE_UNKNOWN)
        .export_values();

    m.def(
        "get_id",
        [](const py::dict &obj) -> py::bytes {
            return CatalogObject::getId(
                       static_cast<SkyObject::TYPE>(py::cast<int>(obj["type"])),
                       py::cast<double>(obj["ra"]), py::cast<double>(obj["dec"]),
                       py::cast<QString>(obj["name"]),
                       py::cast<QString>(obj["catalog_identifier"]))
                .toStdString();
        },
        "object"_a,
        R"(
        Calculate the id of an object.

        Parameters
        ----------)");

    ///////////////////////////////////////////////////////////////////////////
    //                             Sql Statements                            //
    ///////////////////////////////////////////////////////////////////////////
    auto s = m.def_submodule("sqlstatements");
    {
        using namespace CatalogsDB::SqlStatements;

        s.doc() = "Assorted sql statements to modify the catalog database.";

        s.def("insert_dso", &insert_dso, "catalog_id"_a);
        s.def("create_catalog_table", &create_catalog_table, "catalog_id"_a);

#define ATTR(name)            \
    {                         \
        s.attr(#name) = name; \
    }
        ATTR(create_catalog_list_table);
        ATTR(insert_catalog);
        ATTR(get_catalog_by_id);
        ATTR(all_catalog_view);
        ATTR(master_catalog);
        ATTR(dso_by_name);
#undef ATTR
    }
}
