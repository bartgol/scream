#ifndef EAMXX_IO_STREAM_HPP
#ifndef EAMXX_IO_STREAM_HPP

namespace scream
{

class IOStream
{
public:
  using fm_type       = FieldManager;
  using remapper_type = AbstractRemapper;
  using atm_diag_type = AtmosphereDiagnostic;

  using KT = KokkosTypes<DefaultDevice>;
  template<int N>
  using view_Nd_dev  = typename KT::template view_ND<Real,N>;
  template<int N>
  using view_Nd_host = typename KT::template view_ND<Real,N>::HostMirror;

  using view_1d_dev  = view_Nd_dev<1>;
  using view_1d_host = view_Nd_host<1>;

  virtual ~AtmosphereOutput () = default;

  // Constructor
  IOStream (const std::shared_ptr<const fm_type>& field_mgr);

  IOStream (const std::shared_ptr<remapper_type>& remapper);

  IOStream (const ekat::Comm& comm,
            const std::vector<Field>& fields,
            const std::shared_ptr<const grid_type>& grid);

  // Short version for outputing a list of fields (no remapping supported)
  AtmosphereOutput(const ekat::Comm& comm,
                   const std::vector<Field>& fields,
                   const std::shared_ptr<const grid_type>& grid);

  // Main Functions
  void write_fields (const std::string& filename,
                     const int nsteps_since_last_output);
  void read_fields  (const std::string& filename);

  void reset_dev_views();
  void update_avg_cnt_view(const Field&, view_1d_dev& dev_view);
  void setup_output_file (const std::string& filename, const std::string& fp_precision, const scorpio::FileMode mode);

  void set_fp_precision (const std::string& fp_precision);

  long long res_dep_memory_footprint () const;

  std::shared_ptr<const AbstractGrid> get_io_grid () const {
    return m_io_grid;
  }

protected:
  // Internal functions
  void set_grid (const std::shared_ptr<const AbstractGrid>& grid);
  void set_field_manager (const std::shared_ptr<const fm_type>& field_mgr, const std::string& mode);
  void set_field_manager (const std::shared_ptr<const fm_type>& field_mgr, const std::vector<std::string>& modes);

  std::shared_ptr<const fm_type> get_field_manager (const std::string& mode) const;

  void register_dimensions(const std::string& name);
  void register_variables(const std::string& filename, const std::string& fp_precision, const scorpio::FileMode mode);
  void set_degrees_of_freedom(const std::string& filename);
  std::vector<scorpio::offset_t> get_var_dof_offsets (const FieldLayout& layout);
  void register_views();
  Field get_field(const std::string& name, const std::string mode) const;
  void compute_diagnostic (const std::string& name, const bool allow_invalid_fields = false);
  void set_diagnostics();
  void create_diagnostic (const std::string& diag_name);

  // --- Internal variables --- //
  ekat::Comm                          m_comm;

  // We store two shared pointers for field managers:
  // io_field_manager stores the fields in the layout for output
  // sim_field_manager points to the simulation field manager
  // when remapping horizontally these two field managers may be different.
  std::map<std::string,std::shared_ptr<const fm_type>> m_field_mgrs;
  std::shared_ptr<const grid_type>            m_io_grid;
  std::shared_ptr<remapper_type>              m_horiz_remapper;
  std::shared_ptr<remapper_type>              m_vert_remapper;
  std::shared_ptr<const gm_type>              m_grids_manager;

  // How to combine multiple snapshots in the output: Instant, Max, Min, Average
  OutputAvgType     m_avg_type;
  Real              m_avg_coeff_threshold = 0.5; // % of unfilled values required to not just assign value as FillValue

  // Internal maps to the output fields, how the columns are distributed, the file dimensions and the global ids.
  std::vector<std::string>                              m_fields_names;
  std::vector<std::string>                              m_avg_cnt_names;
  std::map<std::string,std::string>                     m_field_to_avg_cnt_map;
  std::map<std::string,std::string>                     m_fields_alt_name;
  std::map<std::string,FieldLayout>                     m_layouts;
  std::map<std::string,int>                             m_dofs;
  std::map<std::string,std::pair<int,bool>>             m_dims;
  std::map<std::string,std::shared_ptr<atm_diag_type>>  m_diagnostics;
  std::map<std::string,std::vector<std::string>>        m_diag_depends_on_diags;
  std::map<std::string,bool>                            m_diag_computed;

  // Use float, so that if output fp_precision=float, this is a representable value.
  // Otherwise, you would get an error from Netcdf, like
  //   NetCDF: Numeric conversion not representable
  // Also, by default, don't pick max float, to avoid any overflow if the value
  // is used inside other calculation and/or remap.
  float m_fill_value = constants::DefaultFillValue<float>().value;

  // Local views of each field to be used for "averaging" output and writing to file.
  std::map<std::string,view_1d_host>    m_host_views_1d;
  std::map<std::string,view_1d_dev>     m_dev_views_1d;
  std::map<std::string,view_1d_dev>     m_local_tmp_avg_cnt_views_1d;
  std::map<std::string,view_1d_dev>     m_avg_coeff_views_1d;

  bool m_add_time_dim;
  bool m_track_avg_cnt = false;
};

} // namespace scream

#endif // EAMXX_IO_STREAM_HPP
