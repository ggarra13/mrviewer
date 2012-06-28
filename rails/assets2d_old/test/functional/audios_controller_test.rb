require File.dirname(__FILE__) + '/../test_helper'
require 'audios_controller'

# Re-raise errors caught by the controller.
class AudiosController; def rescue_action(e) raise e end; end

class AudiosControllerTest < Test::Unit::TestCase
  fixtures :audios

  def setup
    @controller = AudiosController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = audios(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:audios)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:audio)
    assert assigns(:audio).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:audio)
  end

  def test_create
    num_audios = Audio.count

    post :create, :audio => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_audios + 1, Audio.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:audio)
    assert assigns(:audio).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      Audio.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      Audio.find(@first_id)
    }
  end
end
