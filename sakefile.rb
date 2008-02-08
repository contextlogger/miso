# -*- ruby -*-
# sake variant sake0

# A makefile for Miso. Sake must be installed to use this makefile.
# Standard Symbian (and sdk2unix) style makefiles are also provided
# in source releases for those who do not want to install Sake.

require 'sake0/component'

# The UID has been allocated from Symbian.
$comp = Sake::Component.new(:basename => 'miso',
                            :vendor => "HIIT",
                            :version => [1, 93],
                            :uid_v8 => 0x10206ba4,
                            :caps => (%w{AllFiles} + Sake::DEV_CERT_CAPS))

trunk_dir = Pathname.new(File.join(ENV["HOME"], "trunk"))
$comp.web_dir = trunk_dir + "html" + "pdis" + "download" + $comp.basename

if $sake_op[:kits]
  $kits = Sake::DevKits::get_exact_set($sake_op[:kits].strip.split(/,/))
else
  # Target all devices that look like they support Python builds.
  $kits = Sake::DevKits::get_all
  $kits.delete_if do |kit|
    !kit.supports_python?
  end
end

$signsisrc = File.join(ENV["HOME"], "symbian-signed", "signsis.rb")
load($signsisrc) if File.exist? $signsisrc

if $sake_op[:szeged]
  # use Szeged GCC 3.0 backport
  $epoc_gcc = "gcc-3.0-psion-98r2"
end

$builds = $kits.map do |kit|
  Sake::CompBuild.new :component => $comp, :devkit => kit
end
$builds = $builds.map do |build|
  if build.target.symbian_platform.major_version < 9
    build
  else
    # DevCert caps are not all that much use in Miso at present, but
    # in case someone has signed their Python application with those
    # caps, they will need a Miso with at least those caps as well. So
    # we create both self-signed and DevCert-signed variants.

    selfbuild = build.to_self_signed
    selfbuild.cert_file = $sake_op[:cert] || $self_cert || raise
    selfbuild.key_file = $sake_op[:key] || $key_file || raise
    selfbuild.passphrase = $sake_op[:passphrase] || $key_password

    devbuild = build.to_dev_signed
    devbuild.cert_file = $sake_op[:cert] || $dev_cert || raise
    devbuild.key_file = $sake_op[:key] || $key_file || raise
    devbuild.passphrase = $sake_op[:passphrase] || $key_password

    [selfbuild, devbuild]
  end
end
$builds.flatten!

# We probably do not require separate documentation for every single
# build variant, as at least the interface should be just about the
# same in each file; although given that we are documenting private
# stuff, too, there could be differences, but still, let us just pick
# one version.
$doc_build = $builds.last

for build in $builds
  map = build.trait_map

  if $sake_op[:logging] and map[:has_flogger]
    map[:do_logging] = :define
  end

  list = %w{euser.lib python222.lib}
  list += %w{bluetooth.lib efsrv.lib esock.lib hal.lib sysutil.lib}
  list += %w{flogger.lib} if map[:do_logging]
  list += %w{vibractrl.lib} if map[:has_vibractrl] and not map[:has_hwrmvibra]
  list += %w{hwrmvibraclient.lib} if map[:has_hwrmvibra]
  build.libs = list

  list = %w{fs_notify_change.cpp local_epoc_py_utils.cpp module_init.cpp}
  list += %w{vibra.cpp} if map[:has_vibractrl] or map[:has_hwrmvibra]
  list = list.sort.map do |x|
    $comp.src_dir + x
  end
  build.cxx_files = list
end

task :default => [:pyd, :sis]

require 'sake0/tasks'

Sake::Tasks::def_list_devices_tasks(:builds => $builds)

Sake::Tasks::def_makefile_tasks(:builds => $builds)

Sake::Tasks::def_pyd_tasks(:builds => $builds)

Sake::Tasks::def_sis_tasks(:builds => $builds)

Sake::Tasks::def_clean_tasks(:builds => $builds)

task :all => [:makefiles, :pyd]

if $doc_build
  # C++ API documentation.
  Sake::Tasks::def_doxygen_tasks(:build => $doc_build)
  task :all => :cxxdoc

  # Python API documentation.
  py_file = $doc_build.src_dir + ($doc_build.component.basename + ".py")
  Sake::Tasks::def_pydoc_tasks(:build => $doc_build, :py_file => py_file)
  task :all => :pydoc
end

Sake::Tasks::def_dist_tasks :builds => $builds
Sake::Tasks::def_web_tasks :builds => $builds

Sake::Tasks::force_uncurrent_on_op_change
